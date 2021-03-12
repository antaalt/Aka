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
		// Next are only minifying filter
		MipMapNearest,
		MipMapLinear,
	};
	enum class Wrap {
		Clamp,
		Repeat,
		Mirror,
	};
	Filter filterMin;
	Filter filterMag;
	Wrap wrapS;
	Wrap wrapT;

	bool operator==(const Sampler& rhs);
	bool operator!=(const Sampler& rhs);
};

class Texture
{
public:
	using Ptr = std::shared_ptr<Texture>;
	using Handle = StrictType<uintptr_t, struct TextureTagName>;
	enum class Format {
		UnsignedByte,
		Byte,
		UnsignedShort,
		Short,
		UnsignedInt,
		Int,
		Half,
		Float
	};
	enum class Component {
		Red,
		RG,
		RGB,
		BGR,
		RGBA,
		BGRA,
		Depth,
		DepthStencil,
	};

protected:
	Texture(uint32_t width, uint32_t height, Format format, Component component, Sampler sampler);
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	virtual ~Texture();
public:
	static Texture::Ptr create(uint32_t width, uint32_t height, Format format, Component component, Sampler sampler);

	uint32_t width() const;

	uint32_t height() const;

	Format format() const;

	Component component() const;

	const Sampler& sampler() const;

	virtual void upload(const Rect& rect, const void* data) = 0;

	virtual void upload(const void* data) = 0;

	virtual void download(void* data) = 0;

	virtual void copy(Texture::Ptr src, const Rect& rect) = 0;

	virtual Handle handle() = 0;

	virtual bool isFramebuffer() = 0;

protected:
	Sampler m_sampler;
	Format m_format;
	Component m_component;
	uint32_t m_width, m_height;
};

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
