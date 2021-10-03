#include <Aka/Graphic/Texture2DArray.h>

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Core/Application.h>

namespace aka {

Texture2DArray::Texture2DArray(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags) :
	Texture(width, height, 1, TextureType::Texture2DArray, format, flags)
{
}

Texture2DArray::~Texture2DArray()
{
}

Texture2DArray::Ptr Texture2DArray::create(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, uint32_t layerCount, const void** layers)
{
	return nullptr;
}

};