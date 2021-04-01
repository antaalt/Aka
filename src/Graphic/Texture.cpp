#include <Aka/Graphic/Texture.h>

#include <Aka/Graphic/GraphicBackend.h>

namespace aka {

Texture::Texture(uint32_t width, uint32_t height, TextureFormat format, TextureComponent component, TextureFlag flags, Sampler sampler) :
	m_sampler(sampler),
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

Texture::Ptr Texture::create(uint32_t width, uint32_t height, TextureFormat format, TextureComponent component, TextureFlag flags, Sampler sampler)
{
	return GraphicBackend::createTexture(width, height, format, component, flags, sampler);
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

bool Sampler::operator==(const Sampler& rhs)
{
	return filterMin == rhs.filterMin && filterMag == rhs.filterMag && wrapS == rhs.wrapS && wrapT == rhs.wrapT;
}

bool Sampler::operator!=(const Sampler& rhs)
{
	return filterMin != rhs.filterMin || filterMag != rhs.filterMag || wrapS != rhs.wrapS || wrapT != rhs.wrapT;
}

};