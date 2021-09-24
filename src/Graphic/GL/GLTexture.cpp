#if defined(AKA_USE_OPENGL)
#include "GLTexture.h"

#include "GLContext.h"

namespace aka {

GLTexture2D::GLTexture2D(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* data) :
	Texture2D(width, height, format, flags),
	m_textureID(0)
{
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, glComponentInternal(m_format), width, height, 0, glComponent(m_format), glFormat(m_format), data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.f);
	if ((TextureFlag::GenerateMips & flags) == TextureFlag::GenerateMips)
		glGenerateMipmap(GL_TEXTURE_2D);
}
GLTexture2D::~GLTexture2D()
{
	if (m_textureID != 0)
		glDeleteTextures(1, &m_textureID);
}
void GLTexture2D::upload(const void* data, uint32_t level)
{
	upload(Rect{ 0, 0, m_width, m_height }, data, level);
}
void GLTexture2D::upload(const Rect& rect, const void* data, uint32_t level)
{
	AKA_ASSERT(rect.x + rect.w <= m_width, "");
	AKA_ASSERT(rect.y + rect.h <= m_height, "");
	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glTexSubImage2D(GL_TEXTURE_2D, level, (GLint)rect.x, (GLint)rect.y, (GLsizei)rect.w, (GLsizei)rect.h, glComponent(m_format), glFormat(m_format), data);
	if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
		glGenerateMipmap(GL_TEXTURE_2D);
}
void GLTexture2D::download(void* data, uint32_t level)
{
	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glGetTexImage(GL_TEXTURE_2D, level, glComponent(m_format), glFormat(m_format), data);
}
TextureHandle GLTexture2D::handle() const
{
	return TextureHandle(static_cast<uintptr_t>(m_textureID));
}
void GLTexture2D::generateMips()
{
	if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
		return;
	glGenerateMipmap(GL_TEXTURE_2D);
	m_flags = m_flags | TextureFlag::GenerateMips;
}

GLTextureCubeMap::GLTextureCubeMap(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	const void* px, const void* nx,
	const void* py, const void* ny,
	const void* pz, const void* nz
) :
	TextureCubeMap(width, height, format, flags),
	m_textureID(0)
{
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, glComponentInternal(m_format), width, height, 0, glComponent(m_format), glFormat(m_format), px);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, glComponentInternal(m_format), width, height, 0, glComponent(m_format), glFormat(m_format), nx);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, glComponentInternal(m_format), width, height, 0, glComponent(m_format), glFormat(m_format), py);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, glComponentInternal(m_format), width, height, 0, glComponent(m_format), glFormat(m_format), ny);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, glComponentInternal(m_format), width, height, 0, glComponent(m_format), glFormat(m_format), pz);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, glComponentInternal(m_format), width, height, 0, glComponent(m_format), glFormat(m_format), nz);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.f);
	if ((TextureFlag::GenerateMips & flags) == TextureFlag::GenerateMips)
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}
GLTextureCubeMap::~GLTextureCubeMap()
{
	if (m_textureID != 0)
		glDeleteTextures(1, &m_textureID);
}
void GLTextureCubeMap::upload(const void* data, uint32_t layer, uint32_t level)
{
	upload(Rect{ 0, 0, m_width, m_height }, data, layer, level);
}
void GLTextureCubeMap::upload(const Rect& rect, const void* data, uint32_t layer, uint32_t level)
{
	AKA_ASSERT(rect.x + rect.w <= m_width, "");
	AKA_ASSERT(rect.y + rect.h <= m_height, "");
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer, level, (GLint)rect.x, (GLint)rect.y, (GLsizei)rect.w, (GLsizei)rect.h, glComponent(m_format), glFormat(m_format), data);
	if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}
void GLTextureCubeMap::download(void* data, uint32_t layer, uint32_t level)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
	glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer, level, glComponent(m_format), glFormat(m_format), data);
}
TextureHandle GLTextureCubeMap::handle() const
{
	return TextureHandle(static_cast<uintptr_t>(m_textureID));
}
void GLTextureCubeMap::generateMips()
{
	if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
		return;
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	m_flags = m_flags | TextureFlag::GenerateMips;
}

GLTexture2DMultisample::GLTexture2DMultisample(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	uint8_t samples,
	const void* data
) :
	Texture2DMultisample(width, height, format, flags),
	m_textureID(0)
{
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_textureID);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, glComponentInternal(m_format), width, height, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.f);
	if ((TextureFlag::GenerateMips & flags) == TextureFlag::GenerateMips)
		glGenerateMipmap(GL_TEXTURE_2D_MULTISAMPLE);
}
GLTexture2DMultisample::~GLTexture2DMultisample()
{
	if (m_textureID != 0)
		glDeleteTextures(1, &m_textureID);
}
void GLTexture2DMultisample::upload(const void* data)
{
	upload(Rect{ 0, 0, m_width, m_height }, data);
}
void GLTexture2DMultisample::upload(const Rect& rect, const void* data)
{
	AKA_ASSERT(rect.x + rect.w <= m_width, "");
	AKA_ASSERT(rect.y + rect.h <= m_height, "");
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_textureID);
	glTexSubImage2D(GL_TEXTURE_2D_MULTISAMPLE, 0, (GLint)rect.x, (GLint)rect.y, (GLsizei)rect.w, (GLsizei)rect.h, glComponent(m_format), glFormat(m_format), data);
	if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
		glGenerateMipmap(GL_TEXTURE_2D_MULTISAMPLE);
}
void GLTexture2DMultisample::download(void* data)
{
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_textureID);
	glGetTexImage(GL_TEXTURE_2D_MULTISAMPLE, 0, glComponent(m_format), glFormat(m_format), data);
}
TextureHandle GLTexture2DMultisample::handle() const
{
	return TextureHandle(static_cast<uintptr_t>(m_textureID));
}
void GLTexture2DMultisample::generateMips()
{
	if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
		return;
	glGenerateMipmap(GL_TEXTURE_2D_MULTISAMPLE);
	m_flags = m_flags | TextureFlag::GenerateMips;
}

};

#endif