#include "Texture.h"

#include "Graphic.h"

#include <stdexcept>

namespace aka {

// TODO avoid duplication
GLenum getFilter(Sampler::Filter type) {
	switch (type) {
	default:
	case Sampler::Filter::Linear:
		return GL_LINEAR;
	case Sampler::Filter::Nearest:
		return GL_NEAREST;
	}
}

GLenum getFormat(Texture::Format format) {
	switch (format) {
	default:
		throw std::runtime_error("Not implemneted");
	case Texture::Format::Red:
		return GL_RED;
	case Texture::Format::Rgba:
		return GL_RGBA;
	case Texture::Format::Rgba8:
		return GL_RGBA8;
	}
}

Texture::Texture(uint32_t width, uint32_t height, Format internalFormat, Format format, Sampler::Filter filter) :
	Texture(width, height, internalFormat, format,nullptr, filter)
{
}

Texture::Texture(uint32_t width, uint32_t height, Format internalFormat, Format format, const uint8_t* data,  Sampler::Filter filter)
{
	// TODO add filter & type settings
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, getFormat(internalFormat), width, height, 0, getFormat(format), GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, getFilter(filter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, getFilter(filter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Texture::~Texture()
{
	if (m_textureID != 0)
		glDeleteTextures(1, &m_textureID);
}

Texture::Ptr Texture::create(uint32_t width, uint32_t height, Format internalFormat, Format format, Sampler::Filter filter)
{
	return create(width, height, internalFormat, format, nullptr, filter);
}

Texture::Ptr Texture::create(uint32_t width, uint32_t height, Format internalFormat, Format format, const uint8_t* data, Sampler::Filter filter)
{
	return std::make_shared<Texture>(width, height, internalFormat, format, data, filter);
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