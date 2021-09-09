#pragma once

#include <Aka/Graphic/Texture.h>

namespace aka {

class Texture2D : public Texture
{
public:
	using Ptr = std::shared_ptr<Texture2D>;
protected:
	Texture2D(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flag);
	virtual ~Texture2D();
public:
	static Texture2D::Ptr create(
		uint32_t width,
		uint32_t height,
		TextureFormat format,
		TextureFlag flag,
		const void* data = nullptr
	);
	// Upload a region of the texture2D
	virtual void upload(const Rect& rect, const void* data, uint32_t level = 0) = 0;
	// Upload the texture2D
	virtual void upload(const void* data, uint32_t level = 0) = 0;
	// Download the texture2D
	virtual void download(void* data, uint32_t level = 0) = 0;
	// Copy the texture to dst texture
	//virtual void copy(Texture2D::Ptr dst, const Rect& rect) = 0;
};

};