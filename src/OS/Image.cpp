#include <Aka/OS/Image.h>

#include <Aka/OS/Logger.h>
#include <Aka/Core/Config.h>
#include <Aka/Core/BitField.h>
#include <Aka/Memory/Allocator.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8

struct StbAllocation { uint8_t data; };
static_assert(sizeof(StbAllocation) == 1);
#define STBI_MALLOC(sz)           (::aka::mem::getAllocator(::aka::AllocatorMemoryType::Temporary, ::aka::AllocatorCategory::Default).allocate<StbAllocation>(sz))
#define STBI_REALLOC(p,newsz)     (::aka::mem::getAllocator(::aka::AllocatorMemoryType::Temporary, ::aka::AllocatorCategory::Default).reallocate<StbAllocation>(p, newsz))
#define STBI_FREE(p)              (::aka::mem::getAllocator(::aka::AllocatorMemoryType::Temporary, ::aka::AllocatorCategory::Default).deallocate(p))

#include <stb_image.h>
#include <stb_image_write.h>

#include <stdexcept>

namespace aka {

template struct Img<byte_t>;
template struct Img<float>;

#if defined(AKA_ORIGIN_BOTTOM_LEFT)
constexpr const bool defaultFlipImageAtLoad = true;
constexpr const bool defaultFlipImageAtSave = true;
#else
constexpr const bool defaultFlipImageAtLoad = false;
constexpr const bool defaultFlipImageAtSave = false;
#endif

uint32_t getImageComponentCount(ImageComponent _components)
{
	return countBitSet(toMask(_components));
}

template <>
void Img<byte_t>::set(uint32_t x, uint32_t y, const Pixel& pixel)
{
	const uint32_t c = getImageComponentCount(components);
	size_t stride = c * width;
	memcpy(bytes.data() + x * c + stride * y, pixel.data, c);
}

template <>
void Img<float>::set(uint32_t x, uint32_t y, const Pixel& pixel)
{
	const uint32_t c = getImageComponentCount(components);
	size_t stride = sizeof(float) * c * width;
	memcpy(bytes.data() + x * c + stride * y, pixel.data, c * sizeof(float));
}
template <>
Img<byte_t>::Pixel Img<byte_t>::get(uint32_t x, uint32_t y) const
{
	Pixel out;
	const uint32_t c = getImageComponentCount(components);
	size_t stride = c * width;
	memcpy(out.data, bytes.data() + x * c + stride * y, c);
	return out;
}

template <>
Img<float>::Pixel Img<float>::get(uint32_t x, uint32_t y) const
{
	Pixel out;
	const uint32_t c = getImageComponentCount(components);
	size_t stride = sizeof(float) * c * width;
	memcpy(out.data, bytes.data() + x * c + stride * y, c * sizeof(float));
	return out;
}

ImageFileFormat ImageDecoder::getFileFormat(const Path& _path)
{
	return ImageFileFormat::Unknown;
}
ImageFileFormat ImageDecoder::getFileFormat(const Blob& _blob)
{
	return ImageFileFormat::Unknown;
}
Image ImageDecoder::fromDisk(const Path& _path)
{
	int x, y, channel;
	stbi_set_flip_vertically_on_load(defaultFlipImageAtLoad);
	stbi_uc* data = stbi_load(_path.cstr(), &x, &y, &channel, STBI_rgb_alpha);
	if (data == nullptr)
	{
		return Image{};
	}
	else
	{
		Image img{};
		size_t size = x * y * 4;
		img.bytes.resize(size);
		memcpy(img.bytes.data(), data, size);
		img.width = static_cast<uint32_t>(x);
		img.height = static_cast<uint32_t>(y);
		img.components = ImageComponent::RGBA;
		stbi_image_free(data);
		return img;
	}
}
Image ImageDecoder::fromMemory(const Blob& _blob)
{
	return fromMemory((const byte_t*)_blob.data(), _blob.size());
}
Image ImageDecoder::fromMemory(const byte_t* _data, size_t _size)
{
	int x, y, channel;
	stbi_set_flip_vertically_on_load(defaultFlipImageAtLoad);
	stbi_uc* data = stbi_load_from_memory(_data, (int)_size, &x, &y, &channel, STBI_rgb_alpha);
	if (data == nullptr)
	{
		return Image{};
	}
	else
	{
		Image img{};
		size_t size = x * y * 4;
		img.bytes.resize(size);
		memcpy(img.bytes.data(), data, size);
		img.width = static_cast<uint32_t>(x);
		img.height = static_cast<uint32_t>(y);
		img.components = ImageComponent::RGBA;
		stbi_image_free(data);
		return img;
	}
}
ImageHdr ImageDecoder::fromDiskHdr(const Path& _path)
{
	int x, y, channel;
	stbi_set_flip_vertically_on_load(defaultFlipImageAtLoad);
	float* data = stbi_loadf(_path.cstr(), &x, &y, &channel, STBI_rgb_alpha);
	if (data == nullptr)
	{
		return ImageHdr{};
	}
	else
	{
		ImageHdr img{};
		size_t size = x * y * 4;
		img.bytes.resize(size);
		memcpy(img.bytes.data(), data, size * sizeof(float));
		img.width = static_cast<uint32_t>(x);
		img.height = static_cast<uint32_t>(y);
		img.components = ImageComponent::RGBA;
		stbi_image_free(data);
		return img;
	}
}
ImageHdr ImageDecoder::fromMemoryHdr(const Blob& _blob)
{
	return fromMemoryHdr((const byte_t*)_blob.data(), _blob.size());
}
ImageHdr ImageDecoder::fromMemoryHdr(const byte_t* _data, size_t _size)
{
	int x, y, channel;
	stbi_set_flip_vertically_on_load(defaultFlipImageAtLoad);
	float* data = stbi_loadf_from_memory(_data, (int)_size, &x, &y, &channel, STBI_rgb_alpha);
	if (data == nullptr)
	{
		return ImageHdr{};
	}
	else
	{
		ImageHdr img{};
		size_t size = x * y * 4;
		img.bytes.resize(size);
		memcpy(img.bytes.data(), data, size * sizeof(float));
		img.width = static_cast<uint32_t>(x);
		img.height = static_cast<uint32_t>(y);
		img.components = ImageComponent::RGBA;
		stbi_image_free(data);
		return img;
	}
}

bool ImageEncoder::toDisk(const Path& _path, Image& _image, ImageFileFormat _format, ImageQuality _quality)
{
	const uint32_t c = getImageComponentCount(_image.components);
	AKA_ASSERT(_image.bytes.size() == _image.width * _image.height * c, "Invalid size");
	stbi_flip_vertically_on_write(defaultFlipImageAtSave);
	int error = stbi_write_jpg(_path.cstr(), _image.width, _image.height, c, _image.bytes.data(), (uint32_t)_quality * 50);
	return error != 0;
}
Blob ImageEncoder::toMemory(Image& _image, ImageFileFormat _format, ImageQuality _quality)
{
	AKA_NOT_IMPLEMENTED;
	/*const uint32_t c = getImageComponentCount(_image.components);
	AKA_ASSERT(_image.bytes.size() == _image.width * _image.height * c, "Invalid size");
	stbi_flip_vertically_on_write(defaultFlipImageAtSave);
	Blob data;
	int outLength = stbi_write_jpg_to_func([](void* context, void* data, int size) {
		Blob* bytes = reinterpret_cast<Blob*>(context);
		uint8_t* binaryData = static_cast<uint8_t*>(data);
		bytes->insert(bytes->end(), binaryData, binaryData + size);
		}, &data, _image.width, _image.height, c, _image.bytes.data(), (uint32_t)_quality * 50);
	if (data.size() == 0 || outLength == 0)
		return Blob();
	return data;*/
	return Blob();
}
bool ImageEncoder::toDisk(const Path& _path, ImageHdr& _image, ImageFileFormat _format, ImageQuality _quality)
{
	/*AKA_ASSERT(m_format == ImageFormat::Float, "Invalid format");
	AKA_ASSERT(m_bytes.size() == m_width * m_height * m_components * sizeof(float), "Invalid size");
	stbi_flip_vertically_on_write(defaultFlipImageAtSave);
	int error = stbi_write_hdr(path.cstr(), m_width, m_height, m_components, reinterpret_cast<const float*>(m_bytes.data()));
	return (error != 0);*/
	AKA_NOT_IMPLEMENTED;
	return false;
}
Blob ImageEncoder::toMemory(ImageHdr& _image, ImageFileFormat _format, ImageQuality _quality)
{
	/*AKA_ASSERT(m_format == ImageFormat::Float, "Invalid format");
	AKA_ASSERT(m_bytes.size() == m_width * m_height * m_components * sizeof(float), "Invalid size");
	stbi_flip_vertically_on_write(defaultFlipImageAtSave);
	std::vector<uint8_t> data;
	int outLength = stbi_write_hdr_to_func([](void* context, void* data, int size) {
		std::vector<uint8_t>* bytes = reinterpret_cast<std::vector<uint8_t>*>(context);
		uint8_t* binaryData = static_cast<uint8_t*>(data);
		bytes->insert(bytes->end(), binaryData, binaryData + size);
		}, &data, m_width, m_height, m_components, reinterpret_cast<const float*>(m_bytes.data()));
	if (data.size() == 0 || outLength == 0)
		return std::vector<uint8_t>();*/
	AKA_NOT_IMPLEMENTED;
	return Blob();
}

};