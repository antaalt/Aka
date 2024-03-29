#include <Aka/Graphic/Texture2D.h>

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Core/Application.h>

namespace aka {

Texture2D::Texture2D(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags) :
	Texture(width, height, 1, TextureType::Texture2D, format, flags)
{
}

Texture2D::~Texture2D()
{
}

Texture2D::Ptr Texture2D::create(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* data)
{
	return Application::graphic()->createTexture2D(width, height, format, flags, data);
}

};