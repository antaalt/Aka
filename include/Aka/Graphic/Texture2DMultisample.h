#pragma once

#include <Aka/Graphic/Texture.h>

namespace aka {

class Texture2DMultisample : public Texture
{
public:
	using Ptr = std::shared_ptr<Texture2DMultisample>;
protected:
	Texture2DMultisample(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags);
	virtual ~Texture2DMultisample();
public:
	static Texture2DMultisample::Ptr create(
		uint32_t width,
		uint32_t height,
		TextureFormat format,
		TextureFlag flag,
		uint8_t samples,
		const void* data = nullptr
	);
	// Upload a region of the Texture2DMultisample
	virtual void upload(const Rect& rect, const void* data) = 0;
	// Upload the Texture2DMultisample
	virtual void upload(const void* data) = 0;
	// Download the Texture2DMultisample
	virtual void download(void* data) = 0;
};

};