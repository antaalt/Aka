#include "Image.h"

#include "../Logger.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include <stdexcept>

namespace aka {

Image Image::load(const Path& path)
{
	Image image{};
	int x, y, channel;
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
	stbi_image_free(data);
	return image;
}

Image Image::load(const uint8_t* binaries, size_t size)
{
	Image image{};
	int x, y, channel;
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
	stbi_image_free(data);
	return image;
}

Image Image::load(const std::vector<uint8_t>& binaries)
{
	return load(binaries.data(), binaries.size());
}

void Image::save(const Path& path)
{
	int error = stbi_write_jpg(path.c_str(), width, height, 4, bytes.data(), 80);
	if (error == 0)
		Logger::error("Could not save image at path ", path.str());
}

}