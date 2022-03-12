#include <Aka/Graphic/Texture.h>

#include <Aka/Core/Application.h>

#include <type_traits>

namespace aka {

bool has(TextureFlag flags, TextureFlag flag)
{
	return (flags & flag) == flag;
}
TextureFlag operator&(TextureFlag lhs, TextureFlag rhs)
{
	return static_cast<TextureFlag>(
		static_cast<std::underlying_type<TextureFlag>::type>(lhs) & 
		static_cast<std::underlying_type<TextureFlag>::type>(rhs)
	);
}
TextureFlag operator|(TextureFlag lhs, TextureFlag rhs)
{
	return static_cast<TextureFlag>(
		static_cast<std::underlying_type<TextureFlag>::type>(lhs) |
		static_cast<std::underlying_type<TextureFlag>::type>(rhs)
	);
}

bool Texture::hasMips() const
{
	return has(flags, TextureFlag::GenerateMips);
}
bool Texture::hasLayers() const
{
	switch (type)
	{
	default:
	case aka::TextureType::Texture1D:
	case aka::TextureType::Texture2D:
	case aka::TextureType::Texture3D:
	case aka::TextureType::Texture2DMultisample:
		return false;
	case aka::TextureType::TextureCubeMap:
	case aka::TextureType::Texture1DArray:
	case aka::TextureType::Texture2DArray:
	case aka::TextureType::TextureCubeMapArray:
	case aka::TextureType::Texture2DMultisampleArray:
		return layers > 1;
	}
}
bool Texture::isRenderTarget() const
{
	return has(flags, TextureFlag::RenderTarget);
}
bool Texture::isShaderResource() const
{
	return has(flags, TextureFlag::ShaderResource);
}

bool Texture::isColor(TextureFormat format)
{
	switch (format)
	{
	case aka::TextureFormat::R8:
	case aka::TextureFormat::R8U:
	case aka::TextureFormat::R16:
	case aka::TextureFormat::R16U:
	case aka::TextureFormat::R16F:
	case aka::TextureFormat::R32F:
	case aka::TextureFormat::RG8:
	case aka::TextureFormat::RG8U:
	case aka::TextureFormat::RG16U:
	case aka::TextureFormat::RG16:
	case aka::TextureFormat::RG16F:
	case aka::TextureFormat::RG32F:
	case aka::TextureFormat::RGB8:
	case aka::TextureFormat::RGB8U:
	case aka::TextureFormat::RGB16:
	case aka::TextureFormat::RGB16U:
	case aka::TextureFormat::RGB16F:
	case aka::TextureFormat::RGB32F:
	case aka::TextureFormat::RGBA8:
	case aka::TextureFormat::RGBA8U:
	case aka::TextureFormat::RGBA16:
	case aka::TextureFormat::RGBA16U:
	case aka::TextureFormat::RGBA16F:
	case aka::TextureFormat::RGBA32F:
	case aka::TextureFormat::BGRA:
	case aka::TextureFormat::BGRA8:
	case aka::TextureFormat::BGRA16:
	case aka::TextureFormat::BGRA16U:
	case aka::TextureFormat::BGRA16F:
	case aka::TextureFormat::BGRA32F:
		return true;
	default:
		return false;
	}
}
bool Texture::isDepth(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::Depth:
	case TextureFormat::Depth16:
	case TextureFormat::Depth24:
	case TextureFormat::Depth32:
	case TextureFormat::Depth32F:
		return true;
	default:
		return false;
	}
}
bool Texture::isStencil(TextureFormat format)
{
	return false;
}
bool Texture::isDepthStencil(TextureFormat format)
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
bool Texture::hasDepth(TextureFormat format)
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
bool Texture::hasStencil(TextureFormat format)
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


uint32_t Texture::size(TextureFormat format)
{
	switch (format)
	{
	case aka::TextureFormat::Unknown:
		break;
	case aka::TextureFormat::R8:
	case aka::TextureFormat::R8U:
		return 1;

	case aka::TextureFormat::R16:
	case aka::TextureFormat::R16U:
	case aka::TextureFormat::R16F:
		return 2;
	case aka::TextureFormat::R32F:
		return 4;
	case aka::TextureFormat::RG8:
	case aka::TextureFormat::RG8U:
		return 2;
	case aka::TextureFormat::RG16U:
	case aka::TextureFormat::RG16:
	case aka::TextureFormat::RG16F:
		return 4;
	case aka::TextureFormat::RG32F:
		return 8;
	case aka::TextureFormat::RGB8:
	case aka::TextureFormat::RGB8U:
		return 3;
	case aka::TextureFormat::RGB16:
	case aka::TextureFormat::RGB16U:
	case aka::TextureFormat::RGB16F:
		return 6;
	case aka::TextureFormat::RGB32F:
		return 12;
	case aka::TextureFormat::RGBA8:
	case aka::TextureFormat::RGBA8U:
		return 4;
	case aka::TextureFormat::RGBA16:
	case aka::TextureFormat::RGBA16U:
	case aka::TextureFormat::RGBA16F:
		return 8;
	case aka::TextureFormat::RGBA32F:
		return 16;
	case aka::TextureFormat::BGRA:
	case aka::TextureFormat::BGRA8:
		return 4;
	case aka::TextureFormat::BGRA16:
	case aka::TextureFormat::BGRA16U:
	case aka::TextureFormat::BGRA16F:
		return 8;
	case aka::TextureFormat::BGRA32F:
		return 16;
	default:
		return 0;
	}
	return 0;
}

Texture* Texture::create2D(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* data)
{
	return Application::app()->graphic()->createTexture(width, height, 1, TextureType::Texture2D, 1, 1, format, flags, &data);
}
Texture* Texture::createCubemap(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* const* data)
{
	return Application::app()->graphic()->createTexture(width, height, 1, TextureType::TextureCubeMap, 1, 6, format, flags, data);
}
Texture* Texture::create2DArray(uint32_t width, uint32_t height, uint32_t layers, TextureFormat format, TextureFlag flags, const void* const* data)
{
	return Application::app()->graphic()->createTexture(width, height, 1, TextureType::Texture2DArray, 1, layers, format, flags, data);
}
void Texture::destroy(Texture* texture)
{
	return Application::app()->graphic()->destroy(texture);
}

};