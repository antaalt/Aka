#pragma once 

#include <Aka/Resource/Archive/ArchiveImage.hpp>
#include <Aka/Resource/Resource/Resource.hpp>

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Renderer/Renderer.hpp>

namespace aka {

class Texture : public Resource {
public:
	Texture();
	Texture(AssetID _id, const String& _name);
	~Texture();

	uint32_t getWidth() const { return m_width; }
	uint32_t getHeight() const { return m_height; }
	gfx::TextureHandle getGfxHandle() const { return m_textureHandle; }
	gfx::TextureType getTextureType() const { return m_textureType; }
	gfx::TextureUsage getTextureUsage() const { return m_textureUsage; }
	gfx::TextureFormat getTextureFormat() const { return m_textureFormat; }
	uint32_t getLayerCount() const { return m_layerCount; }
	uint32_t getMipCount() const { return m_mipCount; }
	TextureID getTextureID() const { return m_textureID; }
private:
	void fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer) override;
	void toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer) override;
	void destroy_internal(AssetLibrary* _library, Renderer* _renderer) override;
private:
	TextureID m_textureID; // Bindless ID
	uint32_t m_width, m_height;
	gfx::TextureHandle m_textureHandle;
	gfx::TextureType m_textureType;
	gfx::TextureUsage m_textureUsage;
	gfx::TextureFormat m_textureFormat;
	uint32_t m_layerCount;
	uint32_t m_mipCount;
};


}