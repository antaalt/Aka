#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Platform.h"

namespace app {

Image Image::load(const char* name)
{
	Image image;
	int x, y, channel;
	stbi_uc* data = stbi_load("data/textureBase.jpg", &x, &y, &channel, STBI_rgb_alpha);
	size_t size = x * y * 4;
	image.bytes.resize(size);
	memcpy(image.bytes.data(), data, size);
	image.width = static_cast<uint32_t>(x);
	image.height = static_cast<uint32_t>(y);
	stbi_image_free(data);
	return image;
}

}