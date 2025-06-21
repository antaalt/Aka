#include <Aka/Graphic/Texture.h>

#include <Aka/Core/Application.h>

#include <type_traits>

namespace aka {
namespace gfx {

Texture::Texture(const char* name, uint32_t width, uint32_t height, uint32_t depth, TextureType type, uint32_t levels, uint32_t layers, TextureFormat format, TextureUsage flags) :
	Resource(name, ResourceType::Texture),
	width(width),
	height(height),
	depth(depth),
	type(type),
	levels(levels),
	layers(layers),
	format(format),
	flags(flags)
{

}

bool Texture::hasMips() const
{
	return has(flags, TextureUsage::GenerateMips);
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
	return has(flags, TextureUsage::RenderTarget);
}
bool Texture::isShaderResource() const
{
	return has(flags, TextureUsage::ShaderResource);
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
	case TextureFormat::Bc1:
	case TextureFormat::Bc2:
	case TextureFormat::Bc3:
	case TextureFormat::Bc4:
	case TextureFormat::Bc5:
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
bool Texture::isCompressed(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::Bc1:
	case TextureFormat::Bc2:
	case TextureFormat::Bc3:
	case TextureFormat::Bc4:
	case TextureFormat::Bc5:
		return true;
	default:
		return false;
	}
}


uint32_t Texture::size(uint32_t width, uint32_t height, TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::Unknown:
		break;
	case TextureFormat::R8:
	case TextureFormat::R8U:
		return width * height;

	case TextureFormat::R16:
	case TextureFormat::R16U:
	case TextureFormat::R16F:
		return width * height * 2;
	case TextureFormat::R32F:
		return width * height * 4;
	case TextureFormat::RG8:
	case TextureFormat::RG8U:
		return width * height * 2;
	case TextureFormat::RG16U:
	case TextureFormat::RG16:
	case TextureFormat::RG16F:
		return width * height * 4;
	case TextureFormat::RG32F:
		return width * height * 8;
	case TextureFormat::RGB8:
	case TextureFormat::RGB8U:
		return width * height * 3;
	case TextureFormat::RGB16:
	case TextureFormat::RGB16U:
	case TextureFormat::RGB16F:
		return width * height * 6;
	case TextureFormat::RGB32F:
		return width * height * 12;
	case TextureFormat::RGBA8:
	case TextureFormat::RGBA8U:
		return width * height * 4;
	case TextureFormat::RGBA16:
	case TextureFormat::RGBA16U:
	case TextureFormat::RGBA16F:
		return width * height * 8;
	case TextureFormat::RGBA32F:
		return width * height * 16;
	case TextureFormat::BGRA:
	case TextureFormat::BGRA8:
		return width * height * 4;
	case TextureFormat::BGRA16:
	case TextureFormat::BGRA16U:
	case TextureFormat::BGRA16F:
		return width * height * 8;
	case TextureFormat::BGRA32F:
		return width* height * 16;
	case TextureFormat::Bc1:
	case TextureFormat::Bc4:
		return ((width + 3) / 4) * ((height + 3) / 4) * 8; // bpe = 8
	case TextureFormat::Bc2:
	case TextureFormat::Bc3:
	case TextureFormat::Bc5:
		return ((width + 3) / 4) * ((height + 3) / 4) * 16; // bpe = 16
	default:
		return 0;
	}
	return 0;
}

};
};