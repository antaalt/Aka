#include "../Texture.h"

#include "GLBackend.h"

#include <stdexcept>

namespace aka {

Texture::Texture(uint32_t width, uint32_t height, Format internalFormat, Format format, Sampler::Filter filter) :
	Texture(width, height, internalFormat, format,nullptr, filter)
{
}

Texture::Texture(uint32_t width, uint32_t height, Format internalFormat, Format format, const uint8_t* data, Sampler::Filter filter):
	m_width(width),
	m_height(height),
	m_textureID(0)
{
	// TODO add filter & type settings
	glGenTextures(1, &m_textureID.value());
	glBindTexture(GL_TEXTURE_2D, m_textureID());
	glTexImage2D(GL_TEXTURE_2D, 0, gl::format(internalFormat), width, height, 0, gl::format(format), GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl::filter(filter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl::filter(filter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Texture::~Texture()
{
	if (m_textureID() != 0)
		glDeleteTextures(1, &m_textureID());
}

Texture::Ptr Texture::create(uint32_t width, uint32_t height, Format internalFormat, Format format, Sampler::Filter filter)
{
	return create(width, height, internalFormat, format, nullptr, filter);
}

Texture::Ptr Texture::create(uint32_t width, uint32_t height, Format internalFormat, Format format, const uint8_t* data, Sampler::Filter filter)
{
	return std::make_shared<Texture>(width, height, internalFormat, format, data, filter);
}

uint32_t Texture::width() const
{
	return m_width;
}

uint32_t Texture::height() const
{
	return m_height;
}

void Texture::bind() const
{
	glBindTexture(GL_TEXTURE_2D, m_textureID());
}

Texture::ID Texture::id() const
{
	return m_textureID;
}

};