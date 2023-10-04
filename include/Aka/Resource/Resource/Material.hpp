#pragma once

#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Core/Container/String.h>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Resource/Resource/Texture.hpp>

namespace aka {

class Renderer;
class AssetLibrary;
struct ArchiveLoadContext;
struct ArchiveSaveContext;
struct RendererMaterial;

class Material : public Resource
{
public:
	Material();
	Material(AssetID _id, const String& _name);
	~Material();
public:
	ResourceHandle<Texture> getAlbedoTexture() const { return m_albedo; }
	ResourceHandle<Texture> getNormalTexture() const { return m_normal; }
	MaterialHandle getMaterialHandle() const { return m_materialHandle; }
private:
	void fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer) override;
	void toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer) override;
	void destroy_internal(AssetLibrary* library, Renderer* _renderer) override;
private:
	ResourceHandle<Texture> m_albedo;
	ResourceHandle<Texture> m_normal;
	MaterialHandle m_materialHandle;
};

}