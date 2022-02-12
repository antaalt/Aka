#pragma once

#include <stdint.h>

#include <Aka/OS/Image.h>

namespace aka {

enum class TextureFlag : uint8_t
{
	None = (1 << 0),
	RenderTarget = (1 << 1),
	ShaderResource = (1 << 2),
	GenerateMips = (1 << 3),
};

bool has(TextureFlag flags, TextureFlag flag);
TextureFlag operator&(TextureFlag lhs, TextureFlag rhs);
TextureFlag operator|(TextureFlag lhs, TextureFlag rhs);

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
	Depth32FStencil8
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
};

struct Texture
{
	void* native; // Native handle to the texture

	uint32_t width; // Width of the texture
	uint32_t height; // Height of the texture
	uint32_t depth; // Depth of the texture

	uint32_t layers; // Layer count
	uint32_t levels; // Mips levels

	TextureFormat format; // Underlying format of the texture
	TextureType type; // Type of the texture
	TextureFlag flags; // Texture flags

	bool hasMips() const;
	bool isRenderTarget() const;
	bool isShaderResource() const;

	static bool isColor(TextureFormat format);
	static bool isDepth(TextureFormat format);
	static bool isStencil(TextureFormat format);
	static bool isDepthStencil(TextureFormat format);
	static bool hasDepth(TextureFormat format);
	static bool hasStencil(TextureFormat format);

	static uint32_t size(TextureFormat format);

	static Texture* create2D(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* data = nullptr);
	static Texture* createCubemap(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* const* data = nullptr);
	static Texture* create2DArray(uint32_t width, uint32_t height, uint32_t layers, TextureFormat format, TextureFlag flags, const void* const* data = nullptr);
	static void destroy(Texture* texture);
};

struct SubTexture
{
	Texture* texture;
	Rect region;
	void update() {} // TODO
};

using TextureHandle = Texture*;

};