#pragma once

#include <vector>

#include <Aka/OS/FileSystem.h>
#include <Aka/Core/Geometry.h>

// D3D / Metal / Consoles origin is top left
// OpenGL / OpenGL ES origin is bottom left
#if defined(AKA_USE_OPENGL)
#define ORIGIN_BOTTOM_LEFT
#elif defined(AKA_USE_D3D11)
#define ORIGIN_TOP_LEFT
#endif

namespace aka {

struct Rect {
	int32_t x;
	int32_t y;
	uint32_t w;
	uint32_t h;
};

struct Image
{
	Image();
	Image(uint32_t width, uint32_t height, uint32_t components);
	Image(uint32_t width, uint32_t height, uint32_t components, const uint8_t* data);

	static Image load(const Path& path);
	static Image load(const uint8_t* binaries, size_t size);
	static Image load(const std::vector<uint8_t>& binaries);

	void save(const Path& path) const;


	void set(uint32_t x, uint32_t y, const color24& color);
	void set(uint32_t x, uint32_t y, const color32& color);
	void set(uint32_t x, uint32_t y, const color3f& color);
	void set(uint32_t x, uint32_t y, const color4f& color);
	color32 get(uint32_t x, uint32_t y) const;
	void clear();
	void flip();

	std::vector<uint8_t> bytes;
	uint32_t width, height;
	uint32_t components;
};

}