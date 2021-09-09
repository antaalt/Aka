#pragma once

#include <stdint.h>
#include <memory>

#include <Aka/Core/StrictType.h>
#include <Aka/Core/Geometry.h>
#include <Aka/OS/Image.h>
#include <Aka/Graphic/Sampler.h>

namespace aka {

enum class TextureFlag : uint8_t {
	None = (1 << 0),
	RenderTarget = (1 << 1),
	ShaderResource = (1 << 2),
	GenerateMips = (1 << 3),
};

enum class TextureFormat : uint8_t {
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

enum class TextureType {
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

// Get the size in bytes of given format
uint32_t size(TextureFormat format);
// Return true if given type is a depth or a depth stencil format
bool isDepth(TextureFormat format);

using TextureHandle = StrictType<uintptr_t, struct TextureHandleTag>;

class Texture
{
public:
	using Ptr = std::shared_ptr<Texture>;
protected:
	Texture(uint32_t width, uint32_t height, uint32_t depth, TextureType type, TextureFormat format, TextureFlag flag);
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	virtual ~Texture();
public:
	// Get width of the texture
	uint32_t width() const;
	// Get height of the texture
	uint32_t height() const;
	// Get depth of the texture
	uint32_t depth() const;
	// Get levels of the texture
	uint32_t levels() const;
	// Get format of the texture
	TextureFormat format() const;
	// Get creation flags of the texture
	TextureFlag flags() const;
	// Get type of the texture
	TextureType type() const;
	// Get handle of the texture
	virtual TextureHandle handle() const = 0;
	// Generate mip maps for the texture.
	virtual void generateMips() = 0;
protected:
	TextureType m_type;
	TextureFormat m_format;
	TextureFlag m_flags;
	uint32_t m_width, m_height, m_depth;
};

TextureFlag operator&(TextureFlag lhs, TextureFlag rhs);
TextureFlag operator|(TextureFlag lhs, TextureFlag rhs);

struct SubTexture
{
	Rect region;
	Texture::Ptr texture;

	// Update the uv values depending on region
	void update();
	// Get the uv value
	const uv2f& get(uint32_t uv) const;
private:
	uv2f m_uv[2];
};

}
