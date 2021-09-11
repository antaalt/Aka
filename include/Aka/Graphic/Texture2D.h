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

	// Copy the whole texture to dst
	static void copy(const Texture2D::Ptr& src, const Texture2D::Ptr& dst, uint32_t level = 0);
	// Copy the texture region to dst
	static void copy(const Texture2D::Ptr& src, const Texture2D::Ptr& dst, const Rect& rectSRC, const Rect& rectDST, uint32_t level = 0);
	// Blit the whole texture to dst
	static void blit(const Texture2D::Ptr& src, const Texture2D::Ptr& dst, TextureFilter filter, uint32_t level = 0);
	// Blit the texture region to dst region
	static void blit(const Texture2D::Ptr& src, const Texture2D::Ptr& dst, const Rect& rectSRC, const Rect& rectDST, TextureFilter filter, uint32_t level = 0);
protected:
	// Copy the texture to dst texture
	virtual void copy(const Texture2D::Ptr& dst, const Rect& rectSRC, const Rect& rectDST, uint32_t level = 0) = 0;
	// Blit the texture to dst texture
	virtual void blit(const Texture2D::Ptr& dst, const Rect& rectSRC, const Rect& rectDST, TextureFilter filter, uint32_t level = 0) = 0;
};

};