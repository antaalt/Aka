#pragma once

#include <Aka/Graphic/Texture.h>

namespace aka {

class Texture2DArray : public Texture
{
public:
	using Ptr = std::shared_ptr<Texture2DArray>;
protected:
	Texture2DArray(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flag);
	virtual ~Texture2DArray();
public:
	static Texture2DArray::Ptr create(
		uint32_t width,
		uint32_t height,
		TextureFormat format,
		TextureFlag flag,
		uint32_t layerCount,
		const void** layers = nullptr
	);
	// Upload a region of the TextureCubeFace
	virtual void upload(const Rect& rect, const void* data, uint32_t layer, uint32_t level = 0) = 0;
	// Upload the TextureCubeFace
	virtual void upload(const void* data, uint32_t layer, uint32_t level = 0) = 0;
	// Download the TextureCubeFace
	virtual void download(void* data, uint32_t layer, uint32_t level = 0) = 0;
};

};