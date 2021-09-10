#pragma once

#include <vector>

#include <Aka/OS/FileSystem.h>
#include <Aka/Core/Geometry.h>

// D3D / Metal / Consoles origin convention is top left
// OpenGL / OpenGL ES origin convention is bottom left
// Aka use top left as default for everything because its easier.
#if !defined(AKA_ORIGIN_TOP_LEFT) && !defined(AKA_ORIGIN_BOTTOM_LEFT)
#define AKA_ORIGIN_TOP_LEFT
#endif

namespace aka {

struct Rect {
	int32_t x;
	int32_t y;
	uint32_t w;
	uint32_t h;
};

#if defined(AKA_ORIGIN_BOTTOM_LEFT)
constexpr const bool defaultFlipImageAtLoad = true;
constexpr const bool defaultFlipImageAtSave = true;
#else
constexpr const bool defaultFlipImageAtLoad = false;
constexpr const bool defaultFlipImageAtSave = false;
#endif

enum class ImageFormat {
	None,
	UnsignedByte,
	Float,
};

class Image
{
public:
	Image();
	Image(uint32_t width, uint32_t height, uint32_t components, ImageFormat format);
	Image(uint32_t width, uint32_t height, uint32_t components, const uint8_t* data);
	Image(uint32_t width, uint32_t height, uint32_t components, const float* data);

	// Load an image and decode it
	static Image load(const Path& path);
	// Load an image and decode it
	static Image load(const uint8_t* binaries, size_t size);
	// Load an image and decode it
	static Image loadHDR(const Path& path);
	// Load an image and decode it
	static Image loadHDR(const uint8_t* binaries, size_t size);

	// Decode an image from path.
	bool decode(const Path& path);
	// Decode an image from bytes.
	bool decode(const uint8_t* binaries, size_t size);
	// Decode an image from path.
	bool decodeHDR(const Path& path);
	// Decode an image from bytes.
	bool decodeHDR(const uint8_t* binaries, size_t size);
	// Encode the image in JPG on the stream.
	bool encodeJPG(const Path& path, uint32_t quality) const;
	std::vector<uint8_t> encodeJPG(uint32_t quality) const;
	// Encode the image in PNG on the stream.
	bool encodePNG(const Path& path) const;
	std::vector<uint8_t> encodePNG() const;
	// Encode the image in HDR on the stream.
	bool encodeHDR(const Path& path) const;
	std::vector<uint8_t> encodeHDR() const;

	// Set a pixel of the image
	template <typename T>
	void set(uint32_t x, uint32_t y, const color3<T>& color);
	// Set a pixel of the image
	template <typename T>
	void set(uint32_t x, uint32_t y, const color4<T>& color);
	// Get a pixel of the image
	template <typename T>
	color4<T> get(uint32_t x, uint32_t y) const;
	// Clear the image data
	void clear();
	// Flip the image on the y axis
	void flip();

	// Get width of the image
	uint32_t width() const;
	// Get height of the image
	uint32_t height() const;
	// Get components of the image
	uint32_t components() const;
	// Get the image format.
	ImageFormat format() const;
	// Get raw data 
	void* data();
	// Get raw data 
	const void* data() const;
	// Get data size
	size_t size() const;

private:
	uint32_t m_width, m_height;
	uint32_t m_components;
	ImageFormat m_format;
	std::vector<uint8_t> m_bytes;
};

template <typename T>
inline void Image::set(uint32_t x, uint32_t y, const color3<T>& color)
{
	AKA_ASSERT(m_components > 3, "Cannot set color3 to image.");
	if constexpr (std::is_same<T, float>::value)
	{
		size_t stride = sizeof(float) * m_components * m_width;
		memcpy(m_bytes.data() + x * sizeof(float) * m_components + stride * y, color.data, 3 * sizeof(float));
	}
	else if constexpr (std::is_same<T, uint8_t>::value)
	{
		size_t stride = m_components * m_width;
		memcpy(m_bytes.data() + x * m_components + stride * y, color.data, 3);
	}
	else
	{
		static_assert(false, "Type not supported");
	}
}
template <typename T>
inline void Image::set(uint32_t x, uint32_t y, const color4<T>& color)
{
	AKA_ASSERT(m_components == 4, "Cannot set color4 to image.");
	if constexpr (std::is_same<T, float>::value)
	{
		size_t stride = sizeof(float) * m_components * m_width;
		memcpy(m_bytes.data() + x * sizeof(float) * m_components + stride * y, color.data, 4 * sizeof(float));
	}
	else if constexpr (std::is_same<T, uint8_t>::value)
	{
		size_t stride = m_components * m_width;
		memcpy(m_bytes.data() + x * m_components + stride * y, color.data, 4);
	}
	else
	{
		static_assert(false, "Type not supported");
	}
}
template <typename T>
inline color4<T> Image::get(uint32_t x, uint32_t y) const
{
	color4<T> out{};
	if constexpr (std::is_same<T, float>::value)
	{
		size_t stride = sizeof(float) * m_components * m_width;
		memcpy(out.data, m_bytes.data() + x * sizeof(float) * m_components + stride * y, 4 * sizeof(float));
	}
	else if constexpr (std::is_same<T, uint8_t>::value)
	{
		size_t stride = m_components * m_width;
		memcpy(out.data, m_bytes.data() + x * m_components + stride * y, 4);
	}
	else
	{
		static_assert(false, "Type not supported");
	}
	return out;
}

}