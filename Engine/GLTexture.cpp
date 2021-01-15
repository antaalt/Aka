#include "Texture.h"

#include "Graphic.h"

namespace aka {

Texture::Texture(uint32_t width, uint32_t height, Sampler::Filter filter) :
	Texture(width, height, nullptr, filter)
{
}

Texture::Texture(uint32_t width, uint32_t height, const void* data, Sampler::Filter filter)
{
	// TODO add filter & type settings
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Texture::~Texture()
{
	if (m_textureID != 0)
		glDeleteTextures(1, &m_textureID);
}

Texture::Ptr Texture::create(uint32_t width, uint32_t height, Sampler::Filter filter)
{
	return create(width, height, nullptr, filter);
}

Texture::Ptr Texture::create(uint32_t width, uint32_t height, const void* data, Sampler::Filter filter)
{
	return std::make_shared<Texture>(width, height, data, filter);
}

void Texture::bind() const
{
	glBindTexture(GL_TEXTURE_2D, m_textureID);
}

Texture::ID Texture::id() const
{
	return m_textureID;
}

};