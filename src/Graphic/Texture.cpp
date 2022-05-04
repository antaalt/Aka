#include <Aka/Graphic/Texture.h>

#include <Aka/Core/Application.h>

#include <type_traits>

namespace aka {
namespace gfx {

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
	case TextureType::Texture1D:
	case TextureType::Texture2D:
	case TextureType::Texture3D:
	case TextureType::Texture2DMultisample:
		return false;
	case TextureType::TextureCubeMap:
	case TextureType::Texture1DArray:
	case TextureType::Texture2DArray:
	case TextureType::TextureCubeMapArray:
	case TextureType::Texture2DMultisampleArray:
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
	case TextureFormat::R8:
	case TextureFormat::R8U:
	case TextureFormat::R16:
	case TextureFormat::R16U:
	case TextureFormat::R16F:
	case TextureFormat::R32F:
	case TextureFormat::RG8:
	case TextureFormat::RG8U:
	case TextureFormat::RG16U:
	case TextureFormat::RG16:
	case TextureFormat::RG16F:
	case TextureFormat::RG32F:
	case TextureFormat::RGB8:
	case TextureFormat::RGB8U:
	case TextureFormat::RGB16:
	case TextureFormat::RGB16U:
	case TextureFormat::RGB16F:
	case TextureFormat::RGB32F:
	case TextureFormat::RGBA8:
	case TextureFormat::RGBA8U:
	case TextureFormat::RGBA16:
	case TextureFormat::RGBA16U:
	case TextureFormat::RGBA16F:
	case TextureFormat::RGBA32F:
	case TextureFormat::BGRA:
	case TextureFormat::BGRA8:
	case TextureFormat::BGRA16:
	case TextureFormat::BGRA16U:
	case TextureFormat::BGRA16F:
	case TextureFormat::BGRA32F:
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
	case TextureFormat::Unknown:
		break;
	case TextureFormat::R8:
	case TextureFormat::R8U:
		return 1;

	case TextureFormat::R16:
	case TextureFormat::R16U:
	case TextureFormat::R16F:
		return 2;
	case TextureFormat::R32F:
		return 4;
	case TextureFormat::RG8:
	case TextureFormat::RG8U:
		return 2;
	case TextureFormat::RG16U:
	case TextureFormat::RG16:
	case TextureFormat::RG16F:
		return 4;
	case TextureFormat::RG32F:
		return 8;
	case TextureFormat::RGB8:
	case TextureFormat::RGB8U:
		return 3;
	case TextureFormat::RGB16:
	case TextureFormat::RGB16U:
	case TextureFormat::RGB16F:
		return 6;
	case TextureFormat::RGB32F:
		return 12;
	case TextureFormat::RGBA8:
	case TextureFormat::RGBA8U:
		return 4;
	case TextureFormat::RGBA16:
	case TextureFormat::RGBA16U:
	case TextureFormat::RGBA16F:
		return 8;
	case TextureFormat::RGBA32F:
		return 16;
	case TextureFormat::BGRA:
	case TextureFormat::BGRA8:
		return 4;
	case TextureFormat::BGRA16:
	case TextureFormat::BGRA16U:
	case TextureFormat::BGRA16F:
		return 8;
	case TextureFormat::BGRA32F:
		return 16;
	default:
		return 0;
	}
	return 0;
}

TextureHandle Texture::create2D(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* data)
{
	return Application::app()->graphic()->createTexture(width, height, 1, TextureType::Texture2D, 1, 1, format, flags, &data);
}
TextureHandle Texture::createCubemap(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* const* data)
{
	return Application::app()->graphic()->createTexture(width, height, 1, TextureType::TextureCubeMap, 1, 6, format, flags, data);
}
TextureHandle Texture::create2DArray(uint32_t width, uint32_t height, uint32_t layers, TextureFormat format, TextureFlag flags, const void* const* data)
{
	return Application::app()->graphic()->createTexture(width, height, 1, TextureType::Texture2DArray, 1, layers, format, flags, data);
}
void Texture::destroy(TextureHandle texture)
{
	return Application::app()->graphic()->destroy(texture);
}

};
};