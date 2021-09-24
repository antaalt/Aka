#include <Aka/Graphic/Texture2DMultisample.h>

#include <Aka/Graphic/GraphicBackend.h>

namespace aka {

Texture2DMultisample::Texture2DMultisample(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags) :
	Texture(width, height, 1, TextureType::Texture2DMultisample, format, flags)
{
}

Texture2DMultisample::~Texture2DMultisample()
{
}

Texture2DMultisample::Ptr Texture2DMultisample::create(
	uint32_t width,
	uint32_t height,
	TextureFormat format,
	TextureFlag flags,
	uint8_t samples,
	const void* data
)
{
	return GraphicBackend::device()->createTexture2DMultisampled(width, height, format, flags, samples, data);
}

};