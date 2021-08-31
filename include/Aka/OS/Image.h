#pragma once

#include <vector>

#include <Aka/OS/FileSystem.h>
#include <Aka/Core/Geometry.h>

// D3D / Metal / Consoles origin is top left
// OpenGL / OpenGL ES origin is bottom left
#if defined(AKA_USE_OPENGL)
#define AKA_ORIGIN_BOTTOM_LEFT
#elif defined(AKA_USE_D3D11)
#define AKA_ORIGIN_TOP_LEFT
#endif

namespace aka {

struct Rect {
	int32_t x;
	int32_t y;
	uint32_t w;
	uint32_t h;
};

struct Image;
struct ImageHDR;

#if defined(AKA_ORIGIN_BOTTOM_LEFT)
constexpr const bool defaultFlipImageAtLoad = true;
constexpr const bool defaultFlipImageAtSave = true;
#else
constexpr const bool defaultFlipImageAtLoad = false;
constexpr const bool defaultFlipImageAtSave = false;
#endif

struct Image
{
	Image();
	Image(uint32_t width, uint32_t height, uint32_t components);
	Image(uint32_t width, uint32_t height, uint32_t components, const uint8_t* data);
	Image(ImageHDR&);

	static Image load(const Path& path, bool flip = defaultFlipImageAtLoad);
	static Image load(const uint8_t* binaries, size_t size, bool flip = defaultFlipImageAtLoad);
	static Image load(const std::vector<uint8_t>& binaries, bool flip = defaultFlipImageAtLoad);

	void save(const Path& path, bool flip = defaultFlipImageAtSave) const;
	std::vector<uint8_t> save(bool flip = defaultFlipImageAtSave) const;

	void set(uint32_t x, uint32_t y, const color24& color);
	void set(uint32_t x, uint32_t y, const color32& color);
	color32 get(uint32_t x, uint32_t y) const;
	void clear();
	void flip();

	std::vector<uint8_t> bytes;
	uint32_t width, height;
	uint32_t components;
};

struct ImageHDR
{
	ImageHDR();
	ImageHDR(uint32_t width, uint32_t height, uint32_t components);
	ImageHDR(uint32_t width, uint32_t height, uint32_t components, const float* data);
	ImageHDR(Image&);

	static ImageHDR load(const Path& path, bool flip = defaultFlipImageAtLoad);
	static ImageHDR load(const uint8_t* binaries, size_t size, bool flip = defaultFlipImageAtLoad);
	static ImageHDR load(const std::vector<uint8_t>& binaries, bool flip = defaultFlipImageAtLoad);

	void save(const Path& path, bool flip = defaultFlipImageAtSave) const;

	void set(uint32_t x, uint32_t y, const color3f& color);
	void set(uint32_t x, uint32_t y, const color4f& color);
	color4f get(uint32_t x, uint32_t y) const;
	void clear();
	void flip();

	std::vector<float> bytes;
	uint32_t width, height;
	uint32_t components;
};

}