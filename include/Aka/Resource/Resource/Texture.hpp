#pragma once 

#include <Aka/Resource/Archive/ArchiveImage.hpp>
#include <Aka/Resource/Resource/Resource.hpp>

#include <Aka/Graphic/GraphicDevice.h>

namespace aka {

class Renderer;

class Texture : public Resource {
public:
	Texture();
	Texture(AssetID _id, const String& _name);

	uint32_t getWidth() const { return m_width; }
	uint32_t getHeight() const { return m_height; }
	gfx::TextureHandle getGfxHandle() const { return m_textureHandle; }
	gfx::TextureType getTextureType() const { return m_textureType; }
	gfx::TextureUsage getTextureUsage() const { return m_textureUsage; }
	gfx::TextureFormat getTextureFormat() const { return m_textureFormat; }
	uint32_t getLayerCount() const { return m_layerCount; }
	uint32_t getMipCount() const { return m_mipCount; }
private:
	void create_internal(AssetLibrary* _library, Renderer* _renderer, const Archive& _archive) override;
	void save_internal(AssetLibrary* _library, Renderer* _renderer, Archive& _archive) override;
	void destroy_internal(AssetLibrary* _library, Renderer* _renderer) override;
private:
	uint32_t m_width, m_height;
	gfx::TextureHandle m_textureHandle;
	gfx::TextureType m_textureType;
	gfx::TextureUsage m_textureUsage;
	gfx::TextureFormat m_textureFormat;
	uint32_t m_layerCount;
	uint32_t m_mipCount;
};


}