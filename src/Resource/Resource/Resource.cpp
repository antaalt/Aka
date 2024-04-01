#include <Aka/Resource/Resource/Resource.hpp>

#include <Aka/Renderer/Renderer.hpp>

namespace aka {

Resource::Resource(ResourceType _type) :
	Resource(_type, AssetID::Invalid, "")
{
}
Resource::Resource(ResourceType _type, AssetID id, const aka::String& _name) :
	m_type(_type), 
	m_id(id), 
	m_name(_name),
	m_state(ResourceState::Disk)
{
}

void Resource::fromArchive(ArchiveLoadContext& _context, Renderer* _renderer)
{
	AKA_ASSERT(m_state == ResourceState::Disk, "Trying to load a resource that is not on disk state");
	m_state = ResourceState::Pending;
	fromArchive_internal(_context, _renderer);
	m_state = ResourceState::Loaded;
}

void Resource::toArchive(ArchiveSaveContext& _context, Renderer* _renderer)
{
	AKA_ASSERT(m_state == ResourceState::Loaded, "Trying to save resource that is not loaded");
	toArchive_internal(_context, _renderer);
}

void Resource::destroy(AssetLibrary* _library, Renderer* _renderer)
{
	AKA_ASSERT(m_state == ResourceState::Loaded, "Trying to destroy resource that is not loaded");
	m_state = ResourceState::Pending;
	destroy_internal(_library, _renderer);
	m_state = ResourceState::Disk;
}

const char* getResourceTypeString(ResourceType type)
{
	switch (type)
	{

	case ResourceType::Scene:
		return "Scene";
	case ResourceType::StaticMesh:
		return "StaticMesh";
	case ResourceType::Material:
		return "Material";
	case ResourceType::Texture:
		return "Texture";
	case ResourceType::AudioSource:
		return "AudioSource";
	default:
		AKA_ASSERT(false, "Invalid resource type");
		return nullptr;
	}
}

}