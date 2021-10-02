#include <Aka/OS/Image.h>

#include <Aka/OS/Logger.h>
#include <Aka/Core/Debug.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>
#include <stb_image_write.h>

#include <stdexcept>

namespace aka {

#if defined(AKA_ORIGIN_BOTTOM_LEFT)
constexpr const bool defaultFlipImageAtLoad = true;
constexpr const bool defaultFlipImageAtSave = true;
#else
constexpr const bool defaultFlipImageAtLoad = false;
constexpr const bool defaultFlipImageAtSave = false;
#endif

size_t size(ImageFormat format)
{
	switch (format)
	{
	default:
	case aka::ImageFormat::None:
		return 0;
	case aka::ImageFormat::UnsignedByte:
		return 1;
	case aka::ImageFormat::Float:
		return 4;
	}
}

Image::Image() :
	Image(0, 0, 0, ImageFormat::None)
{
}

Image::Image(uint32_t width, uint32_t height, uint32_t components, ImageFormat format) :
	m_width(width),
	m_height(height),
	m_components(components),
	m_format(format),
	m_bytes(width * height * components * aka::size(format))
{
}

Image::Image(uint32_t width, uint32_t height, uint32_t components, const float* data) :
	m_width(width),
	m_height(height),
	m_components(components),
	m_format(ImageFormat::Float),
	m_bytes((uint8_t*)data, (uint8_t*)data + width * height * components * sizeof(float))
{
}

Image::Image(uint32_t width, uint32_t height, uint32_t components, const uint8_t* data) :
	m_width(width),
	m_height(height),
	m_components(components),
	m_format(ImageFormat::UnsignedByte),
	m_bytes(data, data + width * height * components)
{
}

Image Image::load(const Path& path)
{
	Image image;
	image.decode(path);
	return image;
}

Image Image::load(const uint8_t* binaries, size_t size)
{
	Image image;
	image.decode(binaries, size);
	return image;
}

Image Image::loadHDR(const Path& path)
{
	Image image;
	image.decodeHDR(path);
	return image;
}

Image Image::loadHDR(const uint8_t* binaries, size_t size)
{
	Image image;
	image.decodeHDR(binaries, size);
	return image;
}

bool Image::decode(const Path& path)
{
	Image image{};
	int x, y, channel;
	stbi_set_flip_vertically_on_load(defaultFlipImageAtLoad);
	stbi_uc* data = stbi_load(path.cstr(), &x, &y, &channel, STBI_rgb_alpha);
	if (data == nullptr)
	{
		Logger::error("Could not load image from path ", path.cstr());
		return false;
	}
	else
	{
		size_t size = x * y * 4;
		m_bytes.resize(size);
		memcpy(m_bytes.data(), data, size);
		m_width = static_cast<uint32_t>(x);
		m_height = static_cast<uint32_t>(y);
		m_components = 4;
		m_format = ImageFormat::UnsignedByte;
		stbi_image_free(data);
		return true;
	}
}

bool Image::decode(const uint8_t* binaries, size_t size)
{
	Image image{};
	int x, y, channel;
	stbi_set_flip_vertically_on_load(defaultFlipImageAtLoad);
	stbi_uc* data = stbi_load_from_memory(binaries, (int)size, &x, &y, &channel, STBI_rgb_alpha);
	if (data == nullptr)
	{
		Logger::error("Could not load image from binaries");
		return false;
	}
	else
	{
		size_t size = x * y * 4;
		m_bytes.resize(size);
		memcpy(m_bytes.data(), data, size);
		m_width = static_cast<uint32_t>(x);
		m_height = static_cast<uint32_t>(y);
		m_components = 4;
		m_format = ImageFormat::UnsignedByte;
		stbi_image_free(data);
		return true;
	}
}

bool Image::decodeHDR(const Path& path)
{
	Image image{};
	int x, y, channel;
	stbi_set_flip_vertically_on_load(defaultFlipImageAtLoad);
	float* data = stbi_loadf(path.cstr(), &x, &y, &channel, STBI_rgb_alpha);
	if (data == nullptr)
	{
		Logger::error("Could not load hdr image from path ", path.cstr());
		return false;
	}
	else
	{
		size_t size = x * y * 4 * sizeof(float);
		m_bytes.resize(size);
		memcpy(m_bytes.data(), data, size);
		m_width = static_cast<uint32_t>(x);
		m_height = static_cast<uint32_t>(y);
		m_components = 4;
		m_format = ImageFormat::Float;
		stbi_image_free(data);
		return true;
	}
}

bool Image::decodeHDR(const uint8_t* binaries, size_t size)
{
	Image image{};
	int x, y, channel;
	stbi_set_flip_vertically_on_load(defaultFlipImageAtLoad);
	float* data = stbi_loadf_from_memory(binaries, (int)size, &x, &y, &channel, STBI_rgb_alpha);
	if (data == nullptr)
	{
		Logger::error("Could not load hdr image from binaries");
		return false;
	}
	else
	{
		size_t size = x * y * 4 * sizeof(float);
		m_bytes.resize(size);
		memcpy(m_bytes.data(), data, size);
		m_width = static_cast<uint32_t>(x);
		m_height = static_cast<uint32_t>(y);
		m_components = 4;
		m_format = ImageFormat::Float;
		stbi_image_free(data);
		return true;
	}
}

bool Image::encodeJPG(const Path& path, uint32_t quality) const
{
	AKA_ASSERT(m_format == ImageFormat::UnsignedByte, "Invalid format");
	AKA_ASSERT(m_bytes.size() == m_width * m_height * m_components, "Invalid size");
	stbi_flip_vertically_on_write(defaultFlipImageAtSave);
	int error = stbi_write_jpg(path.cstr(), m_width, m_height, m_components, m_bytes.data(), quality);
	return error != 0;
}

std::vector<uint8_t> Image::encodeJPG(uint32_t quality) const
{
	AKA_ASSERT(m_format == ImageFormat::UnsignedByte, "Invalid format");
	AKA_ASSERT(m_bytes.size() == m_width * m_height * m_components, "Invalid size");
	stbi_flip_vertically_on_write(defaultFlipImageAtSave);
	std::vector<uint8_t> data;
	int outLength = stbi_write_jpg_to_func([](void* context, void* data, int size) {
		std::vector<uint8_t>* bytes = reinterpret_cast<std::vector<uint8_t>*>(context);
		uint8_t* binaryData = static_cast<uint8_t*>(data);
		bytes->insert(bytes->end(), binaryData, binaryData + size);
	}, &data, m_width, m_height, m_components, m_bytes.data(), quality);
	if (data.size() == 0 || outLength == 0)
		return std::vector<uint8_t>();
	return data;
}

bool Image::encodePNG(const Path& path) const
{
	AKA_ASSERT(m_format == ImageFormat::UnsignedByte, "Invalid format");
	AKA_ASSERT(m_bytes.size() == m_width * m_height * m_components, "Invalid size");
	stbi_flip_vertically_on_write(defaultFlipImageAtSave);
	int error = stbi_write_png(path.cstr(), m_width, m_height, m_components, m_bytes.data(), m_width * m_components);
	return error != 0;
}

std::vector<uint8_t> Image::encodePNG() const
{
	AKA_ASSERT(m_format == ImageFormat::UnsignedByte, "Invalid format");
	AKA_ASSERT(m_bytes.size() == m_width * m_height * m_components, "Invalid size");
	stbi_flip_vertically_on_write(defaultFlipImageAtSave);
	int outLength = 0;
	unsigned char* data = stbi_write_png_to_mem(m_bytes.data(), m_width * m_components, m_width, m_height, m_components, &outLength);
	if (data == nullptr || outLength == 0)
		return std::vector<uint8_t>();
	std::vector<uint8_t> bytes(data, data + outLength);
	free(data);
	return bytes;
}

bool Image::encodeHDR(const Path& path) const
{
	AKA_ASSERT(m_format == ImageFormat::Float, "Invalid format");
	AKA_ASSERT(m_bytes.size() == m_width * m_height * m_components * sizeof(float), "Invalid size");
	stbi_flip_vertically_on_write(defaultFlipImageAtSave);
	int error = stbi_write_hdr(path.cstr(), m_width, m_height, m_components, reinterpret_cast<const float*>(m_bytes.data()));
	return (error != 0);
}

std::vector<uint8_t> Image::encodeHDR() const
{
	AKA_ASSERT(m_format == ImageFormat::Float, "Invalid format");
	AKA_ASSERT(m_bytes.size() == m_width * m_height * m_components * sizeof(float), "Invalid size");
	stbi_flip_vertically_on_write(defaultFlipImageAtSave);
	std::vector<uint8_t> data;
	int outLength = stbi_write_hdr_to_func([](void* context, void* data, int size) {
		std::vector<uint8_t>* bytes = reinterpret_cast<std::vector<uint8_t>*>(context);
		uint8_t* binaryData = static_cast<uint8_t*>(data);
		bytes->insert(bytes->end(), binaryData, binaryData + size);
	}, &data, m_width, m_height, m_components, reinterpret_cast<const float*>(m_bytes.data()));
	if (data.size() == 0 || outLength == 0)
		return std::vector<uint8_t>();
	return data;
}

void Image::clear()
{
	m_width = 0;
	m_height = 0;
	m_components = 0;
	m_format = ImageFormat::None;
	m_bytes.clear();
}

void Image::flip()
{
	std::vector<uint8_t> tmp(m_bytes.size());
	size_t componentSize = aka::size(m_format);
	size_t stride = componentSize * m_width * m_components;
	for (uint32_t i = 0; i < m_height; i++)
		memcpy(tmp.data() + (m_height - 1 - i) * stride, m_bytes.data() + i * stride, stride);
	m_bytes = tmp;
}

uint32_t Image::width() const
{
	return m_width;
}

uint32_t Image::height() const
{
	return m_height;
}

uint32_t Image::components() const
{
	return m_components;
}

ImageFormat Image::format() const
{
	return m_format;
}

void* Image::data()
{
	return m_bytes.data();
}

const void* Image::data() const
{
	return m_bytes.data();
}

size_t Image::size() const
{
	return m_bytes.size();
}

};