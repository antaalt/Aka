#pragma once

#include <stdint.h>
#include <memory>

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
	using ID = uint32_t;
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

	void bind() const;
	ID id() const;

private:
	ID m_textureID;
};

}
