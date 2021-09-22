#include <Aka/Graphic/Texture.h>

#include <Aka/Graphic/GraphicBackend.h>

namespace aka {


uint32_t size(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::R8: return 1;
	case TextureFormat::R8U: return 1;
	case TextureFormat::R16: return 2;
	case TextureFormat::R16U: return 2;
	case TextureFormat::R16F: return 2;
	case TextureFormat::R32F: return 4;

	case TextureFormat::RG8: return 2;
	case TextureFormat::RG8U: return 2;
	case TextureFormat::RG16: return 4;
	case TextureFormat::RG16U: return 4;
	case TextureFormat::RG16F: return 4;
	case TextureFormat::RG32F: return 8;

	case TextureFormat::RGB8: return 3;
	case TextureFormat::RGB8U: return 3;
	case TextureFormat::RGB16: return 6;
	case TextureFormat::RGB16U: return 6;
	case TextureFormat::RGB16F: return 6;
	case TextureFormat::RGB32F: return 12;

	case TextureFormat::RGBA8: return 4;
	case TextureFormat::RGBA8U: return 4;
	case TextureFormat::RGBA16: return 8;
	case TextureFormat::RGBA16U: return 8;
	case TextureFormat::RGBA16F: return 8;
	case TextureFormat::RGBA32F: return 16;

	case TextureFormat::Depth: return 4; // ?
	case TextureFormat::Depth16: return 2; // ?
	case TextureFormat::Depth24: return 3; // ?
	case TextureFormat::Depth32: return 4; // ?
	case TextureFormat::Depth32F: return 4; // ?
	case TextureFormat::DepthStencil: return 4; // ?
	case TextureFormat::Depth0Stencil8: return 1; // ?
	case TextureFormat::Depth24Stencil8: return 4; // ?
	case TextureFormat::Depth32FStencil8: return 4; // ?
		
	default: return 0;
	}
}

bool isDepth(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::Depth:
	case TextureFormat::Depth16:
	case TextureFormat::Depth24:
	case TextureFormat::Depth32:
	case TextureFormat::Depth32F:
	case TextureFormat::DepthStencil:
	case TextureFormat::Depth0Stencil8:
	case TextureFormat::Depth24Stencil8:
	case TextureFormat::Depth32FStencil8:
		return true;
	default:
		return false;
	}
}

bool isDepthStencil(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::DepthStencil:
	case TextureFormat::Depth0Stencil8:
	case TextureFormat::Depth24Stencil8:
	case TextureFormat::Depth32FStencil8:
		return true;
	default:
		return false;
	}
}

Texture::Texture(uint32_t width, uint32_t height, uint32_t depth, TextureType type, TextureFormat format, TextureFlag flags) :
	m_type(type),
	m_format(format),
	m_flags(flags),
	m_width(width),
	m_height(height),
	m_depth(depth)
{
}

Texture::~Texture()
{
}

uint32_t Texture::width() const
{
	return m_width;
}

uint32_t Texture::height() const
{
	return m_height;
}

uint32_t Texture::depth() const
{
	return m_depth;
}

uint32_t Texture::levels() const
{
	if ((m_flags & TextureFlag::GenerateMips) == TextureFlag::GenerateMips)
		return TextureSampler::mipLevelCount(m_width, m_height); // TODO cache
	return 1;
}

TextureFormat Texture::format() const
{
	return m_format;
}

TextureFlag Texture::flags() const
{
	return m_flags;
}

TextureType Texture::type() const
{
	return m_type;
}

void Texture::copy(const Texture::Ptr& src, const Texture::Ptr& dst)
{
	TextureRegion region{ 0, 0, min(src->width(), dst->width()), min(src->height(), dst->height()), 0, 0 };
	GraphicBackend::copy(src, dst, region, region);
}

void Texture::copy(const Texture::Ptr& src, const Texture::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST)
{
	GraphicBackend::copy(src, dst, regionSRC, regionDST);
}

void Texture::blit(const Texture::Ptr& src, const Texture::Ptr& dst, TextureFilter filter)
{
	TextureRegion regionSRC{ 0, 0, src->width(), src->height(), 0, 0 };
	TextureRegion regionDST{ 0, 0, dst->width(), dst->height(), 0, 0 };
	GraphicBackend::blit(src, dst, regionSRC, regionDST, filter);
}

void Texture::blit(const Texture::Ptr& src, const Texture::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST, TextureFilter filter)
{
	GraphicBackend::blit(src, dst, regionSRC, regionDST, filter);
}

TextureFlag operator~(TextureFlag value)
{
	return static_cast<TextureFlag>(~static_cast<std::underlying_type<TextureFlag>::type>(value));
}

TextureFlag operator&(TextureFlag lhs, TextureFlag rhs)
{
	return static_cast<TextureFlag>(static_cast<std::underlying_type<TextureFlag>::type>(lhs) & static_cast<std::underlying_type<TextureFlag>::type>(rhs));
}

TextureFlag operator|(TextureFlag lhs, TextureFlag rhs)
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

};