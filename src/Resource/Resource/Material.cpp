#include <Aka/Resource/Resource/Material.hpp>

#include <Aka/Resource/Archive/ArchiveMaterial.hpp>
#include <Aka/Resource/AssetLibrary.hpp>
#include <Aka/Renderer/Renderer.hpp>

namespace aka {

Material::Material() :
	Resource(ResourceType::Material, AssetID::Invalid, ""),
	m_material(nullptr)
{
}
Material::Material(AssetID id, const String& _name) :
	Resource(ResourceType::Material, id, _name),
	m_material(nullptr)
{
}
Material::~Material()
{
}

void Material::fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer)
{
	const ArchiveMaterial& archive = _context.getArchive<ArchiveMaterial>(getID());
	m_albedo = _context.getAssetLibrary()->load<Texture>(archive.albedo, _renderer);
	m_normal = _context.getAssetLibrary()->load<Texture>(archive.normal, _renderer);
	archive.flags; // TODO
	m_material = _renderer->createMaterialData();
	Memory::copy(m_material->data.color, archive.color.data, sizeof(float) * 4);
	m_material->data.albedoID = EnumToValue(m_albedo.get().getTextureID());
	m_material->data.normalID = EnumToValue(m_normal.get().getTextureID());
}

void Material::toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer)
{
	ArchiveMaterial& archive = _context.getArchive<ArchiveMaterial>(getID());
	AKA_NOT_IMPLEMENTED;
}

void Material::destroy_internal(AssetLibrary* _library, Renderer* _renderer)
{
	_renderer->destroyMaterialData(m_material);
	m_albedo.reset();
	m_normal.reset();
}

}