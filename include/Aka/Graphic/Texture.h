#pragma once

#include <stdint.h>

#include <Aka/Graphic/Resource.h>
#include <Aka/OS/Image.h>
#include <Aka/Core/Enum.h>

namespace aka {
namespace gfx {

enum class TextureUsage : uint8_t
{
	Unknown			= 0,

	None			= 1 << 0,
	RenderTarget	= 1 << 1,
	ShaderResource	= 1 << 2,
	Storage			= 1 << 3,
	GenerateMips	= 1 << 4,

	First			= None,
	Last			= GenerateMips,
};
AKA_IMPLEMENT_BITMASK_OPERATOR(TextureUsage);

enum class TextureFormat : uint8_t
{
	Unknown,

	R8,
	R8U,
	R16,
	R16U,
	R16F,
	R32F,

	RG8,
	RG8U,
	RG16U,
	RG16,
	RG16F,
	RG32F,

	RGB8,
	RGB8U,
	RGB16,
	RGB16U,
	RGB16F,
	RGB32F,

	RGBA8,
	RGBA8U,
	RGBA16,
	RGBA16U,
	RGBA16F,
	RGBA32F,

	BGRA,
	BGRA8,
	BGRA16,
	BGRA16U,
	BGRA16F,
	BGRA32F,

	Depth,
	Depth16,
	Depth24,
	Depth32,
	Depth32F,

	DepthStencil,
	Depth0Stencil8,
	Depth24Stencil8,
	Depth32FStencil8,

	First = R8,
	Last = Depth32FStencil8,
};

enum class TextureType : uint8_t
{
	Unknown,

	Texture1D,
	Texture2D,
	Texture3D,
	TextureCubeMap,
	Texture1DArray,
	Texture2DArray,
	TextureCubeMapArray,
	Texture2DMultisample, // no mip map
	Texture2DMultisampleArray, // no mip map

	First = Texture1D,
	Last = Texture2DMultisampleArray,
};

struct Texture;
using TextureHandle = ResourceHandle<Texture>;

struct Texture : Resource
{
	Texture(const char* name, uint32_t width, uint32_t height, uint32_t depth, TextureType type, uint32_t levels, uint32_t layers, TextureFormat format, TextureUsage flags);

	uint32_t width; // Width of the texture
	uint32_t height; // Height of the texture
	uint32_t depth; // Depth of the texture

	uint32_t layers; // Layer count
	uint32_t levels; // Mips levels

	TextureFormat format; // Underlying format of the texture
	TextureType type; // Type of the texture
	TextureUsage flags; // Texture flags

	bool hasMips() const;
	bool hasLayers() const;
	bool isRenderTarget() const;
	bool isShaderResource() const;

	static bool isColor(TextureFormat format);
	static bool isDepth(TextureFormat format);
	static bool isStencil(TextureFormat format);
	static bool isDepthStencil(TextureFormat format);
	static bool hasDepth(TextureFormat format);
	static bool hasStencil(TextureFormat format);

	static uint32_t size(TextureFormat format);

	static TextureHandle create2D(const char* name, uint32_t width, uint32_t height, TextureFormat format, TextureUsage flags, const void* data = nullptr);
	static TextureHandle createCubemap(const char* name, uint32_t width, uint32_t height, TextureFormat format, TextureUsage flags, const void* const* data = nullptr);
	static TextureHandle create2DArray(const char* name, uint32_t width, uint32_t height, uint32_t layers, TextureFormat format, TextureUsage flags, const void* const* data = nullptr);
	static void destroy(TextureHandle texture);
};

struct SubTexture
{
	TextureHandle texture;
	Rect region;
};

};
};