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
	using ID = StrictType<uint32_t, struct TextureTagName>;
	enum class Format {
		Red,
		Rgba,
		Rgba8,
		Rgba16,
		Rgba32f,
		Rgb8,
		Rgb16,
	};
public:
	Texture(uint32_t width, uint32_t height, Format internalFormat, Format format, Sampler::Filter filter);
	Texture(uint32_t width, uint32_t height, Format internalFormat, Format format, const uint8_t* data, Sampler::Filter filter);
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	~Texture();

	static Texture::Ptr create(uint32_t width, uint32_t height, Format internalFormat, Format format, Sampler::Filter filter);
	static Texture::Ptr create(uint32_t width, uint32_t height, Format internalFormat, Format format, const uint8_t* data, Sampler::Filter filter);

	uint32_t width() const;

	uint32_t height() const;

	void bind() const;

	ID id() const;

private:
	uint32_t m_width, m_height;
	ID m_textureID;
};

}
