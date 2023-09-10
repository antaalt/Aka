#pragma once

#include <vector>

#include <Aka/OS/Path.h>
#include <Aka/Core/Enum.h>
#include <Aka/Core/Geometry.h>
#include <Aka/Core/Container/Vector.h>

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

enum class ImageComponent : uint32_t
{
	None  = 0,

	Red   = 1 << 0,
	Green = 1 << 1,
	Blue  = 1 << 2,
	Alpha = 1 << 3,

	RG    = Red | Green,
	RGB   = Red | Green | Blue,
	RGBA  = Red | Green | Blue | Alpha,
};
AKA_IMPLEMENT_BITMASK_OPERATOR(ImageComponent);

uint32_t getImageComponentCount(ImageComponent _components);

enum class ImageFileFormat
{
	Unknown,

	Jpeg,
	Png,
	Tga,
	Hdr,

	First = Jpeg,
	Last = Hdr,
};

enum class ImageQuality
{
	Low,
	Medium,
	High,
};

template <typename T>
struct Img
{
	using Type = T;

	struct Pixel {
		Pixel() : Pixel(0, 0, 0, 0){}
		Pixel(T r, T g, T b, T a) : data{ r, g, b, a } {}
		T data[4];
	};

	Img();
	Img(uint32_t width, uint32_t height, ImageComponent components);
	Img(uint32_t width, uint32_t height, ImageComponent components, const T* data);

	void set(uint32_t x, uint32_t y, const Pixel& pixel);
	Pixel get(uint32_t x, uint32_t y) const;
	void clear();
	void flip();

	T* data();
	const T* data() const;
	size_t size() const;
	uint32_t getComponents() const;

	uint32_t width, height;
	ImageComponent components;
	Vector<T> bytes;
};


using Image = Img<byte_t>;
using ImageHdr = Img<float>;


struct ImageDecoder
{
	static ImageFileFormat getFileFormat(const Path& _path);
	static ImageFileFormat getFileFormat(const Blob& _blob);
	static Image fromDisk(const Path& _path);
	static Image fromMemory(const Blob& _blob);
	static Image fromMemory(const byte_t* _data, size_t _size);
	static ImageHdr fromDiskHdr(const Path& _path);
	static ImageHdr fromMemoryHdr(const Blob& _blob);
	static ImageHdr fromMemoryHdr(const byte_t* _data, size_t _size);
};

struct ImageEncoder
{
	static bool toDisk(const Path& _path, Image& _image, ImageFileFormat _format, ImageQuality _quality);
	static Blob toMemory(const Path& _path, Image& _image, ImageFileFormat _format, ImageQuality _quality);
	static bool toDisk(const Path& _path, ImageHdr& _image, ImageFileFormat _format, ImageQuality _quality);
	static Blob toMemory(const Path& _path, ImageHdr& _image, ImageFileFormat _format, ImageQuality _quality);
};


template<typename T>
Img<T>::Img()
{
}
template<typename T>
Img<T>::Img(uint32_t width, uint32_t height, ImageComponent components) :
	width(width),
	height(height),
	components(components),
	bytes(width * height* getImageComponentCount(components))
{
}
template<typename T>
Img<T>::Img(uint32_t width, uint32_t height, ImageComponent components, const T* data) :
	width(width),
	height(height),
	components(components),
	bytes(data, width * height * getImageComponentCount(components))
{
}
template<> void Image::set(uint32_t x, uint32_t y, const Pixel& pixel);
template<> void ImageHdr::set(uint32_t x, uint32_t y, const Pixel& pixel);
template<> Image::Pixel Image::get(uint32_t x, uint32_t y) const;
template<> ImageHdr::Pixel ImageHdr::get(uint32_t x, uint32_t y) const;


template<typename T>
void Img<T>::clear()
{
	width = 0;
	height = 0;
	components = ImageComponent::None;
	bytes.clear();
}

template<typename T>
void Img<T>::flip()
{
	// TODO could flip without a copy here.
	const uint32_t c = getImageComponentCount(components);
	Vector<T> flipped(bytes.size());
	size_t stride = width * c * sizeof(T);
	for (uint32_t i = 0; i < height; i++)
		memcpy(flipped.data() + (height - 1 - i) * stride, bytes.data() + i * stride, stride);
	bytes = std::move(flipped);
}

template<typename T>
inline T* Img<T>::data()
{
	return bytes.data();
}

template<typename T>
inline const T* Img<T>::data() const
{
	return bytes.data();
}

template<typename T>
inline size_t Img<T>::size() const
{
	return bytes.size();
}

template<typename T>
inline uint32_t Img<T>::getComponents() const
{
	return getImageComponentCount(components);
}


}