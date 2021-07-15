#include "..\..\include\Aka\Graphic\Texture.h"
#include <Aka/Graphic/Texture.h>

#include <Aka/Graphic/GraphicBackend.h>

namespace aka {

Texture::Texture(uint32_t width, uint32_t height, TextureType type, TextureFormat format, TextureComponent component, TextureFlag flags, Sampler sampler) :
	m_sampler(sampler),
	m_type(type),
	m_format(format),
	m_flags(flags),
	m_component(component),
	m_width(width),
	m_height(height)
{
}

Texture::~Texture()
{
}

Texture::Ptr Texture::create2D(
	uint32_t width, uint32_t height, 
	TextureFormat format, TextureComponent component, TextureFlag flags, 
	Sampler sampler, 
	void* data
)
{
	return GraphicBackend::createTexture2D(width, height, format, component, flags, sampler, data);
}

Texture::Ptr Texture::create2DMultisampled(
	uint32_t width, uint32_t height, 
	TextureFormat format, TextureComponent component, TextureFlag flag, Sampler sampler, 
	void* data, uint8_t samples
)
{
	return GraphicBackend::createTexture2DMultisampled(width, height, format, component, flag, sampler, data, samples);
}

Texture::Ptr Texture::createCubemap(
	uint32_t width, uint32_t height, 
	TextureFormat format, TextureComponent component, TextureFlag flags, 
	Sampler sampler, 
	void* px, void* nx,
	void* py, void* ny,
	void* pz, void* nz
)
{
	return GraphicBackend::createTextureCubeMap(width, height, format, component, flags, sampler, px, nx, py, ny, pz, nz);
}

uint32_t Texture::width() const
{
	return m_width;
}

uint32_t Texture::height() const
{
	return m_height;
}

TextureFormat Texture::format() const
{
	return m_format;
}

TextureComponent Texture::component() const
{
	return m_component;
}

TextureFlag Texture::flags() const
{
	return m_flags;
}

TextureType Texture::type() const
{
	return m_type;
}

const Sampler& Texture::sampler() const
{
	return m_sampler;
}

TextureFlag aka::operator&(TextureFlag lhs, TextureFlag rhs)
{
	return static_cast<TextureFlag>(static_cast<std::underlying_type<TextureFlag>::type>(lhs) & static_cast<std::underlying_type<TextureFlag>::type>(rhs));
}

TextureFlag aka::operator|(TextureFlag lhs, TextureFlag rhs)
{
	return static_cast<TextureFlag>(static_cast<std::underlying_type<TextureFlag>::type>(lhs) | static_cast<std::underlying_type<TextureFlag>::type>(rhs));
}

void SubTexture::update()
{
	if (texture->width() == 0 || texture->height() == 0)
		return;
	if (region.x == 0 ) m_uv[0].u = 0.f;
	else m_uv[0].u = 1.f / (texture->width() / (float)region.x);
	if (region.y == 0) m_uv[0].v = 0.f;
	else m_uv[0].v = 1.f / (texture->height() / (float)region.y);
	
	if (region.w == 0) m_uv[1].u = m_uv[0].u;
	else m_uv[1].u = m_uv[0].u + 1.f / (texture->width() / (float)region.w);
	if (region.h == 0) m_uv[1].v = m_uv[0].v;
	else m_uv[1].v = m_uv[0].v + 1.f / (texture->height() / (float)region.h);
}

const uv2f& SubTexture::get(uint32_t uv) const
{
	return m_uv[uv];
}

uint32_t Sampler::mipLevelCount(uint32_t width, uint32_t height)
{
	return static_cast<uint32_t>(floor(log2(max(width, height)))) + 1;
}

Sampler Sampler::nearest()
{
	Sampler s{};
	s.filterMag = Sampler::Filter::Nearest;
	s.filterMin = Sampler::Filter::Nearest;
	s.mipmapMode = Sampler::MipMapMode::None;
	s.wrapU = Sampler::Wrap::Repeat;
	s.wrapV = Sampler::Wrap::Repeat;
	s.wrapW = Sampler::Wrap::Repeat;
	return s;
}

Sampler Sampler::bilinear()
{
	Sampler s{};
	s.filterMag = Sampler::Filter::Linear;
	s.filterMin = Sampler::Filter::Linear;
	s.mipmapMode = Sampler::MipMapMode::Nearest;
	s.wrapU = Sampler::Wrap::Repeat;
	s.wrapV = Sampler::Wrap::Repeat;
	s.wrapW = Sampler::Wrap::Repeat;
	return s;
}

Sampler Sampler::trilinear()
{
	Sampler s{};
	s.filterMag = Sampler::Filter::Linear;
	s.filterMin = Sampler::Filter::Linear;
	s.mipmapMode = Sampler::MipMapMode::Linear;
	s.wrapU = Sampler::Wrap::Repeat;
	s.wrapV = Sampler::Wrap::Repeat;
	s.wrapW = Sampler::Wrap::Repeat;
	return s;
}

bool Sampler::operator==(const Sampler& rhs)
{
	return filterMin == rhs.filterMin && filterMag == rhs.filterMag && mipmapMode == rhs.mipmapMode && wrapU == rhs.wrapU && wrapV == rhs.wrapV && wrapW == rhs.wrapW;
}

bool Sampler::operator!=(const Sampler& rhs)
{
	return filterMin != rhs.filterMin || filterMag != rhs.filterMag || mipmapMode != rhs.mipmapMode || wrapU != rhs.wrapU || wrapV != rhs.wrapV || wrapW != rhs.wrapW;
}

};