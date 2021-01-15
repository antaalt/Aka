#include "Framebuffer.h"

#include <string>
#include <stdexcept>

#include "Graphic.h"
#include "Logger.h"

namespace aka {

GLenum getFramebufferAttachmentType(Framebuffer::AttachmentType type)
{
	switch (type)
	{
	default:
		Logger::warn("Framebuffer attachment type not defined : ", (int)type);
		return GL_COLOR_ATTACHMENT0;
	case Framebuffer::AttachmentType::Color0:
		return GL_COLOR_ATTACHMENT0;
	case Framebuffer::AttachmentType::Color1:
		return GL_COLOR_ATTACHMENT1;
	case Framebuffer::AttachmentType::Color2:
		return GL_COLOR_ATTACHMENT2;
	case Framebuffer::AttachmentType::Color3:
		return GL_COLOR_ATTACHMENT3;
	case Framebuffer::AttachmentType::Depth:
		return GL_DEPTH_ATTACHMENT;
	case Framebuffer::AttachmentType::Stencil:
		return GL_STENCIL_ATTACHMENT;
	}
}

GLenum getType(Framebuffer::Type type) {
	switch (type) {
	case Framebuffer::Type::Read:
		return GL_READ_FRAMEBUFFER;
	case Framebuffer::Type::Draw:
		return GL_DRAW_FRAMEBUFFER;
	default:
	case Framebuffer::Type::Both:
		return GL_FRAMEBUFFER;
	}
}

GLenum getFilterF(Sampler::Filter type) {
	switch (type) {
	default:
	case Sampler::Filter::Linear:
		return GL_LINEAR;
	case Sampler::Filter::Nearest:
		return GL_NEAREST;
	}
}

Framebuffer::Framebuffer(uint32_t width, uint32_t height, Attachment* attachments, size_t count) :
	m_width(width),
	m_height(height)
{
	glGenFramebuffers(1, &m_framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
	for (size_t iAtt = 0; iAtt < count; iAtt++)
	{
		Attachment& att = attachments[iAtt];
		glFramebufferTexture2D(GL_FRAMEBUFFER, getFramebufferAttachmentType(att.type), GL_TEXTURE_2D, att.texture->id(), 0);
	}
	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not created");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer()
{
	if (m_framebufferID != 0)
		glDeleteFramebuffers(1, &m_framebufferID);
}

Framebuffer::Ptr Framebuffer::create(uint32_t width, uint32_t height, Sampler::Filter filter)
{
	Attachment attachment;
	attachment.type = AttachmentType::Color0;
	attachment.texture = Texture::create(width, height, Texture::Format::Rgba8, Texture::Format::Rgba, nullptr, filter);
	return create(width, height, &attachment, 1);
}

Framebuffer::Ptr Framebuffer::create(uint32_t width, uint32_t height, Attachment* attachment, size_t count)
{
	return std::make_shared<Framebuffer>(width, height, attachment, count);
}

void Framebuffer::bind(Framebuffer::Type type)
{
	glBindFramebuffer(getType(type), m_framebufferID);
}

void Framebuffer::unbind(Type type)
{
	glBindFramebuffer(getType(type), 0);
}

Framebuffer::ID Framebuffer::id() const
{
	return m_framebufferID;
}

void Framebuffer::blit(const Rect& srcRect, const Rect& dstRect, Sampler::Filter filter)
{
	glBlitFramebuffer(
		srcRect.start.x, srcRect.start.y, 
		srcRect.end.x, srcRect.end.y, 
		dstRect.start.x, dstRect.start.y, 
		dstRect.end.x, dstRect.end.y, 
		GL_COLOR_BUFFER_BIT, 
		getFilterF(filter)
	);
}

uint32_t Framebuffer::width() const
{
	return m_width;
}

uint32_t Framebuffer::height() const
{
	return m_height;
}

};