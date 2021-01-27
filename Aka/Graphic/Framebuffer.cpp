#include "Framebuffer.h"
#include "GraphicBackend.h"

namespace aka {

Framebuffer::Framebuffer(uint32_t width, uint32_t height) :
	m_width(width),
	m_height(height)
{
}

Framebuffer::~Framebuffer()
{
}

Framebuffer::Ptr Framebuffer::create(uint32_t width, uint32_t height, Sampler::Filter filter)
{
	Attachment attachment;
	attachment.type = AttachmentType::Color0;
	attachment.texture = Texture::create(width, height, Texture::Format::Rgba, nullptr, filter);
	return create(width, height, &attachment, 1);
}

Framebuffer::Ptr Framebuffer::create(uint32_t width, uint32_t height, Attachment* attachment, size_t count)
{
	return GraphicBackend::createFramebuffer(width, height, attachment, count);
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