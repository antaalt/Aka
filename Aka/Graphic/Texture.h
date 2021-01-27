#pragma once

#include <stdint.h>
#include <memory>

#include "../Core/StrictType.h"

namespace aka {

struct Sampler
{
	enum class Filter {
		Nearest,
		Linear,
		Default = Linear
	};
};

class Texture
{
public:
	using Ptr = std::shared_ptr<Texture>;
	using Handle = StrictType<uintptr_t, struct TextureTagName>;
	enum class Format {
		Red,
		Rgba,
		Rgba8,
		Rgba16,
		Rgba32f,
		Rgb8,
		Rgb16,
		DepthStencil,
	};
protected:
	Texture(uint32_t width, uint32_t height);
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	virtual ~Texture();
public:
	static Texture::Ptr create(uint32_t width, uint32_t height, Format format, Sampler::Filter filter);
	static Texture::Ptr create(uint32_t width, uint32_t height, Format format, const uint8_t* data, Sampler::Filter filter);

	uint32_t width() const;

	uint32_t height() const;

	virtual void bind() = 0;

	virtual Handle handle() = 0;

protected:
	uint32_t m_width, m_height;
};

}
