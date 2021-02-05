#pragma once

#include <stdint.h>
#include <memory>

#include "../Core/StrictType.h"
#include "../Core/Geometry.h"

namespace aka {

struct Rect {
	float x;
	float y;
	float w;
	float h;
};

#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
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
		Red,
		Rgb,
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
	static Texture::Ptr create(uint32_t width, uint32_t height, Format format, Sampler sampler);
	static Texture::Ptr create(uint32_t width, uint32_t height, Format format, const uint8_t* data, Sampler sampler);

	uint32_t width() const;

	uint32_t height() const;

	virtual void upload(const Rect& rect, const uint8_t* data) = 0;

	virtual void upload(const uint8_t* data) = 0;

	virtual void download(uint8_t *data) = 0;

	virtual Handle handle() = 0;

	virtual bool isFramebuffer() = 0;

protected:
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
