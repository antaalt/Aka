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
	AttachmentType type = AttachmentType::Color0;
	return create(width, height, &type, 1, filter);
}

Framebuffer::Ptr Framebuffer::create(uint32_t width, uint32_t height, AttachmentType* attachment, size_t count, Sampler::Filter filter)
{
	return GraphicBackend::createFramebuffer(width, height, attachment, count, filter);
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