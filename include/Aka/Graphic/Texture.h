#pragma once

#include <stdint.h>
#include <memory>

#include "../Core/StrictType.h"
#include "../Core/Geometry.h"
#include "../OS/Image.h"

namespace aka {

struct Sampler
{
	enum class Filter {
		Nearest,
		Linear,
	};
	enum class Wrap {
		Repeat,
		Mirror,
		ClampToEdge,
		ClampToBorder,
	};
	enum class MipMapMode {
		None,
		Nearest,
		Linear,
	};
	Filter filterMin;
	Filter filterMag;
	MipMapMode mipmapMode;
	Wrap wrapU;
	Wrap wrapV;
	Wrap wrapW;

	static uint32_t mipLevelCount(uint32_t width, uint32_t height);

	static Sampler nearest();
	static Sampler bilinear();
	static Sampler trilinear();

	bool operator==(const Sampler& rhs);
	bool operator!=(const Sampler& rhs);
};

enum class TextureFlag : uint8_t {
	None = (1 << 0),
	RenderTarget = (1 << 1),
	//ShaderStorage = (1 << 2),
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
	Texture2D,
	Texture2DMultisample,
	TextureCubemap,
};

// Get the size in bytes of given format
uint32_t size(TextureFormat format);
// Return true if given type is a depth or a depth stencil format
bool isDepth(TextureFormat format);

class Texture
{
public:
	using Ptr = std::shared_ptr<Texture>;
	using Handle = StrictType<uintptr_t, struct TextureHandleTag>;
protected:
	Texture(uint32_t width, uint32_t height, TextureType type, TextureFormat format, TextureFlag flag, Sampler sampler);
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	virtual ~Texture();
public:
	static Texture::Ptr create2D(
		uint32_t width, 
		uint32_t height, 
		TextureFormat format,
		TextureFlag flag, 
		Sampler sampler,
		const void* data = nullptr
	);
	static Texture::Ptr create2DMultisampled(
		uint32_t width,
		uint32_t height,
		TextureFormat format,
		TextureFlag flag,
		Sampler sampler,
		const void* data = nullptr,
		uint8_t samples = 4
	);
	static Texture::Ptr createCubemap(
		uint32_t width,
		uint32_t height,
		TextureFormat format,
		TextureFlag flag,
		Sampler sampler,
		const void* px = nullptr, const void* nx = nullptr,
		const void* py = nullptr, const void* ny = nullptr,
		const void* pz = nullptr, const void* nz = nullptr
	);


	uint32_t width() const;

	uint32_t height() const;

	TextureFormat format() const;

	TextureFlag flags() const;

	TextureType type() const;

	const Sampler& sampler() const;

	virtual void upload(const Rect& rect, const void* data) = 0;

	virtual void upload(const void* data) = 0;

	virtual void upload(uint32_t mipLevel, const Rect& rect, const void* data) = 0;

	virtual void download(void* data) = 0;

	virtual void copy(Texture::Ptr src, const Rect& rect) = 0;

	virtual Handle handle() const = 0;

protected:
	Sampler m_sampler;
	TextureType m_type;
	TextureFormat m_format;
	TextureFlag m_flags;
	uint32_t m_width, m_height;
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
