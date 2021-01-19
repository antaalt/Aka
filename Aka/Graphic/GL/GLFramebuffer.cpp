#include "../Framebuffer.h"

#include <string>
#include <stdexcept>

#include "GLbackend.h"
#include "../../Core/Debug.h"
#include "../../Platform/Logger.h"

namespace aka {
Framebuffer::Framebuffer() :
	m_width(0),
	m_height(0),
	m_framebufferID(0)
{
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
		glFramebufferTexture2D(GL_FRAMEBUFFER, gl::attachmentType(att.type), GL_TEXTURE_2D, att.texture->id().value(), 0);
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
	glBindFramebuffer(gl::framebufferType(type), m_framebufferID);
}

Framebuffer::ID Framebuffer::id() const
{
	return m_framebufferID;
}

void Framebuffer::blit(Framebuffer::Ptr dst, const Rect& srcRect, const Rect& dstRect, Sampler::Filter filter)
{
	bind(Type::Read);
	dst->bind(Type::Draw);
	glBlitFramebuffer(
		static_cast<GLint>(srcRect.x),
		static_cast<GLint>(srcRect.y),
		static_cast<GLint>(srcRect.x + srcRect.w), 
		static_cast<GLint>(srcRect.y + srcRect.h),
		static_cast<GLint>(dstRect.x),
		static_cast<GLint>(dstRect.y),
		static_cast<GLint>(dstRect.x + dstRect.w), 
		static_cast<GLint>(dstRect.y + dstRect.h),
		GL_COLOR_BUFFER_BIT, 
		gl::filter(filter)
	);
	dst->bind(Type::Both);
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