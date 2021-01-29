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

void SubTexture::update()
{
	m_uv[0] = uv2f(1.f / (texture->width() / region.x), 1.f / (texture->height() / region.y));
	m_uv[1] = m_uv[0] + uv2f(1.f / (texture->width() / region.w), 1.f / (texture->height() / region.h));
}

const uv2f& SubTexture::get(uint32_t uv) const
{
	return m_uv[uv];
}

};