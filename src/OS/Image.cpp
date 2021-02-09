#include <Aka/OS/Image.h>

#include <Aka/OS/Logger.h>
#include <Aka/Core/Debug.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
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

Image Image::load(const Path& path)
{
	Image image{};
	int x, y, channel;
#if defined(ORIGIN_BOTTOM_LEFT)
	stbi_set_flip_vertically_on_load(true);
#endif
	stbi_uc* data = stbi_load(path.c_str(), &x, &y, &channel, STBI_rgb_alpha);
	if (data == nullptr)
	{
		Logger::error("Could not load image from path ", path.c_str());
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

Image Image::load(const uint8_t* binaries, size_t size)
{
	Image image{};
	int x, y, channel;
#if defined(ORIGIN_BOTTOM_LEFT)
	stbi_set_flip_vertically_on_load(true);
#endif
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

Image Image::load(const std::vector<uint8_t>& binaries)
{
	return load(binaries.data(), binaries.size());
}

void Image::save(const Path& path) const
{
#if defined(ORIGIN_BOTTOM_LEFT)
	stbi_flip_vertically_on_write(true);
#endif
	int error = stbi_write_png(path.c_str(), width, height, 4, bytes.data(), width * 4);
	if (error == 0)
		Logger::error("Could not save image at path ", path.str());
}

void Image::set(uint32_t x, uint32_t y, const color24& color)
{
	bytes[y * width * components + x + 0] = color.x;
	bytes[y * width * components + x + 1] = color.y;
	bytes[y * width * components + x + 2] = color.z;
}

void Image::set(uint32_t x, uint32_t y, const color32& color)
{
	ASSERT(components == 4, "Cannot set color4 to image.");
	bytes[y * width * components + x + 0] = color.r;
	bytes[y * width * components + x + 1] = color.g;
	bytes[y * width * components + x + 2] = color.b;
	bytes[y * width * components + x + 3] = color.a;
}

void Image::set(uint32_t x, uint32_t y, const color3f& color)
{
	bytes[y * width * components + x + 0] = static_cast<uint8_t>(clamp(color.x * 255.f, 0.f, 255.f));
	bytes[y * width * components + x + 1] = static_cast<uint8_t>(clamp(color.y / 255.f, 0.f, 255.f));
	bytes[y * width * components + x + 2] = static_cast<uint8_t>(clamp(color.z / 255.f, 0.f, 255.f));
}

void Image::set(uint32_t x, uint32_t y, const color4f& color)
{
	ASSERT(components == 4, "Cannot set color4 to image.");
	bytes[y * width * components + x + 0] = static_cast<uint8_t>(clamp(color.r / 255.f, 0.f, 255.f));
	bytes[y * width * components + x + 1] = static_cast<uint8_t>(clamp(color.g / 255.f, 0.f, 255.f));
	bytes[y * width * components + x + 2] = static_cast<uint8_t>(clamp(color.b / 255.f, 0.f, 255.f));
	bytes[y * width * components + x + 3] = static_cast<uint8_t>(clamp(color.a / 255.f, 0.f, 255.f));
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

}