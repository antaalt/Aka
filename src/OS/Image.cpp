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

Image::Image() :
	Image(0,0,0)
{
}

Image::Image(uint32_t width, uint32_t height, uint32_t components) :
	Image(width, height, components, nullptr)
{
}

Image::Image(uint32_t width, uint32_t height, uint32_t components, const uint8_t* data) :
	width(width),
	height(height),
	components(components),
	bytes(width * height* components)
{
	if (data != nullptr)
		memcpy(bytes.data(), data, bytes.size());
}
Image::Image(ImageHDR& imageHDR) :
	width(imageHDR.width),
	height(imageHDR.height),
	components(imageHDR.components),
	bytes(imageHDR.bytes.size())
{
	for (uint32_t i = 0; i < width * height * components; i++)
		bytes[i] = static_cast<uint8_t>(clamp(imageHDR.bytes[i] * 255.f, 0.f, 255.f));
}

Image Image::load(const Path& path, bool flip)
{
	Image image{};
	int x, y, channel;
	stbi_set_flip_vertically_on_load(flip);
	stbi_uc* data = stbi_load(path.cstr(), &x, &y, &channel, STBI_rgb_alpha);
	if (data == nullptr)
	{
		Logger::error("Could not load image from path ", path.cstr());
		return image;
	}
	size_t size = x * y * 4;
	image.bytes.resize(size);
	memcpy(image.bytes.data(), data, size);
	image.width = static_cast<uint32_t>(x);
	image.height = static_cast<uint32_t>(y);
	image.components = 4;
	stbi_image_free(data);
	return image;
}

Image Image::load(const uint8_t* binaries, size_t size, bool flip)
{
	Image image{};
	int x, y, channel;
	stbi_set_flip_vertically_on_load(flip);
	stbi_uc* data = stbi_load_from_memory(binaries, (int)size, &x, &y, &channel, STBI_rgb_alpha);
	if (data == nullptr)
	{
		Logger::error("Could not load image from binary");
		return image;
	}
	size_t sizeTexture = x * y * 4;
	image.bytes.resize(sizeTexture);
	memcpy(image.bytes.data(), data, sizeTexture);
	image.width = static_cast<uint32_t>(x);
	image.height = static_cast<uint32_t>(y);
	image.components = 4;
	stbi_image_free(data);
	return image;
}

Image Image::load(const std::vector<uint8_t>& binaries, bool flip)
{
	return load(binaries.data(), binaries.size());
}

void Image::save(const Path& path, bool flip) const
{
	stbi_flip_vertically_on_write(flip);
	int error = stbi_write_png(path.cstr(), width, height, components, bytes.data(), width * components);
	if (error == 0)
		Logger::error("Could not save image at path ", path.str());
}

std::vector<uint8_t> Image::save(bool flip) const
{
	stbi_flip_vertically_on_write(flip);
	int outLength = 0;
	unsigned char* data = stbi_write_png_to_mem(bytes.data(), width * components, width, height, components, &outLength);
	if (data == nullptr || outLength == 0)
	{
		Logger::error("Could not encode image");
		return std::vector<uint8_t>();
	}
	std::vector<uint8_t> bytes(data, data + outLength);
	free(data);
	return bytes;
}

void Image::set(uint32_t x, uint32_t y, const color24& color)
{
	bytes[y * width * components + x + 0] = color.r;
	bytes[y * width * components + x + 1] = color.g;
	bytes[y * width * components + x + 2] = color.b;
}

void Image::set(uint32_t x, uint32_t y, const color32& color)
{
	AKA_ASSERT(components == 4, "Cannot set color4 to image.");
	bytes[y * width * components + x + 0] = color.r;
	bytes[y * width * components + x + 1] = color.g;
	bytes[y * width * components + x + 2] = color.b;
	bytes[y * width * components + x + 3] = color.a;
}

color32 Image::get(uint32_t x, uint32_t y) const
{
	return color32(
		bytes[y * width * components + x + 0],
		bytes[y * width * components + x + 1],
		bytes[y * width * components + x + 2],
		components == 4 ? bytes[y * width * components + x + 0] : 255
	);
}

void Image::clear()
{
	width = 0;
	height = 0;
	components = 0;
	bytes.clear();
}

void Image::flip()
{
	std::vector<uint8_t> tmp(bytes.size());
	for (uint32_t i = 0; i < height; i++)
		memcpy(tmp.data() + (height - 1 - i) * width * components, bytes.data() + i * width * components, width * components);
	bytes = tmp;
}


ImageHDR::ImageHDR() :
	ImageHDR(0, 0, 0)
{
}

ImageHDR::ImageHDR(uint32_t width, uint32_t height, uint32_t components) :
	ImageHDR(width, height, components, nullptr)
{
}

ImageHDR::ImageHDR(uint32_t width, uint32_t height, uint32_t components, const float* data) :
	width(width),
	height(height),
	components(components),
	bytes(width* height* components)
{
	if (data != nullptr)
		memcpy(bytes.data(), data, bytes.size());
}
ImageHDR::ImageHDR(Image& image) :
	width(image.width),
	height(image.height),
	components(image.components),
	bytes(image.bytes.size())
{
	for (uint32_t i = 0; i < width * height * components; i++)
		bytes[i] = image.bytes[i] / 255.f;
}

ImageHDR ImageHDR::load(const Path& path, bool flip)
{
	ImageHDR image{};
	int x, y, channel;
	stbi_set_flip_vertically_on_load(flip);
	float* data = stbi_loadf(path.cstr(), &x, &y, &channel, STBI_rgb_alpha);
	if (data == nullptr)
	{
		Logger::error("Could not load image from path ", path.cstr());
		return image;
	}
	size_t size = x * y * 4 * sizeof(float);
	image.bytes.resize(size);
	memcpy(image.bytes.data(), data, size);
	image.width = static_cast<uint32_t>(x);
	image.height = static_cast<uint32_t>(y);
	image.components = 4;
	stbi_image_free(data);
	return image;
}

ImageHDR ImageHDR::load(const uint8_t* binaries, size_t size, bool flip)
{
	ImageHDR image{};
	int x, y, channel;
	stbi_set_flip_vertically_on_load(flip);
	float* data = stbi_loadf_from_memory(binaries, (int)size, &x, &y, &channel, STBI_rgb_alpha);
	if (data == nullptr)
	{
		Logger::error("Could not load image from binary");
		return image;
	}
	size_t sizeTexture = x * y * 4 * sizeof(float);
	image.bytes.resize(sizeTexture);
	memcpy(image.bytes.data(), data, sizeTexture);
	image.width = static_cast<uint32_t>(x);
	image.height = static_cast<uint32_t>(y);
	image.components = 4;
	stbi_image_free(data);
	return image;
}

ImageHDR ImageHDR::load(const std::vector<uint8_t>& binaries, bool flip)
{
	return load(binaries.data(), binaries.size(), flip);
}

void ImageHDR::save(const Path& path, bool flip) const
{
	stbi_flip_vertically_on_write(flip);
	int error = stbi_write_hdr(path.cstr(), width, height, components, bytes.data());
	if (error == 0)
		Logger::error("Could not save image at path ", path.str());
}

void ImageHDR::set(uint32_t x, uint32_t y, const color3f& color)
{
	bytes[y * width * components + x + 0] = color.r;
	bytes[y * width * components + x + 1] = color.g;
	bytes[y * width * components + x + 2] = color.b;
}

void ImageHDR::set(uint32_t x, uint32_t y, const color4f& color)
{
	AKA_ASSERT(components == 4, "Cannot set color4 to image.");
	bytes[y * width * components + x + 0] = color.r;
	bytes[y * width * components + x + 1] = color.g;
	bytes[y * width * components + x + 2] = color.b;
	bytes[y * width * components + x + 3] = color.a;
}

color4f ImageHDR::get(uint32_t x, uint32_t y) const
{
	return color4f(
		bytes[y * width * components + x + 0],
		bytes[y * width * components + x + 1],
		bytes[y * width * components + x + 2],
		components == 4 ? bytes[y * width * components + x + 0] : 1.f
	);
}

void ImageHDR::clear()
{
	width = 0;
	height = 0;
	components = 0;
	bytes.clear();
}

void ImageHDR::flip()
{
	std::vector<float> tmp(bytes.size());
	for (uint32_t i = 0; i < height; i++)
		memcpy(tmp.data() + (height - 1 - i) * width * components, bytes.data() + i * width * components, width * components);
	bytes = tmp;
}

}