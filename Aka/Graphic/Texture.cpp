#include "Texture.h"

#include "GraphicBackend.h"

namespace aka {

Texture::Texture(uint32_t width, uint32_t height) :
	m_width(width),
	m_height(height)
{
}

Texture::~Texture()
{
}

Texture::Ptr Texture::create(uint32_t width, uint32_t height, Format format, Sampler::Filter filter)
{
	return create(width, height, format, nullptr, filter);
}

Texture::Ptr Texture::create(uint32_t width, uint32_t height, Format format, const uint8_t* data, Sampler::Filter filter)
{
	return GraphicBackend::createTexture(width, height, format, data, filter);
}

uint32_t Texture::width() const
{
	return m_width;
}

uint32_t Texture::height() const
{
	return m_height;
}

};