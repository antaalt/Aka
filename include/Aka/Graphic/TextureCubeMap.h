#pragma once

#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Texture2D.h>

namespace aka {

class TextureCubeMap : public Texture
{
public:
	using Ptr = std::shared_ptr<TextureCubeMap>;
protected:
	TextureCubeMap(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags);
	virtual ~TextureCubeMap();
public:
	// Create a cubemap
	static TextureCubeMap::Ptr create(
		uint32_t width,
		uint32_t height,
		TextureFormat format,
		TextureFlag flags,
		const void* px = nullptr, const void* nx = nullptr,
		const void* py = nullptr, const void* ny = nullptr,
		const void* pz = nullptr, const void* nz = nullptr
	);
	// Generate a cubemap from an equirectangular map
	static TextureCubeMap::Ptr generate(
		uint32_t width,
		uint32_t height,
		TextureFormat format,
		TextureFlag flags,
		Texture2D::Ptr equirectangular,
		TextureFilter filter
	);

	// Upload a region of the layer of the cube map
	virtual void upload(const Rect& rect, const void* data, uint32_t layer, uint32_t level = 0) = 0;
	// Upload the layer of the cube map
	virtual void upload(const void* data, uint32_t layer, uint32_t level = 0) = 0;
	// Download the layer of the cube map
	virtual void download(void* data, uint32_t layer, uint32_t level = 0) = 0;
	// Copy the texture to dst texture
	//virtual void copy(TextureCubeMap::Ptr dst, const Rect& rect) = 0;
};

};