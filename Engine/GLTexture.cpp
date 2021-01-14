#include "Texture.h"

#include "Graphic.h"

namespace aka {

Texture::Texture() :
	m_textureID(0)
{
}

Texture::~Texture()
{
	if (m_textureID != 0)
		destroy();
}

void Texture::create(uint32_t width, uint32_t height, const void* data)
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

void Texture::destroy()
{
	glDeleteTextures(1, &m_textureID);
	m_textureID = 0;
}

void Texture::bind() const
{
	glBindTexture(GL_TEXTURE_2D, m_textureID);
}

TextureID Texture::getID()
{
	return m_textureID;
}

};