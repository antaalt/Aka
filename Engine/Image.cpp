#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Platform.h"

#include <stdexcept>

namespace app {

Image Image::load(const Path& path)
{
	Image image;
	int x, y, channel;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* data = stbi_load(path.c_str(), &x, &y, &channel, STBI_rgb_alpha);
	if (data == nullptr)
		throw std::runtime_error("Could not load image at " + path.str());
	size_t size = x * y * 4;
	image.bytes.resize(size);
	memcpy(image.bytes.data(), data, size);
	image.width = static_cast<uint32_t>(x);
	image.height = static_cast<uint32_t>(y);
	stbi_image_free(data);
	return image;
}

}