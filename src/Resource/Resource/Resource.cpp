#include <Aka/Resource/Resource/Resource.hpp>

namespace aka {

Resource::Resource(ResourceType _type) :
	Resource(_type, ResourceID::Invalid, "") 
{
}
Resource::Resource(ResourceType _type, ResourceID id, const aka::String& _name) :
	m_type(_type), 
	m_id(id), 
	m_name(_name),
	m_state(ResourceState::Disk)
{
}

void Resource::create(AssetLibrary* _library, aka::gfx::GraphicDevice* _device, const Archive& _archive)
{
	AKA_ASSERT(m_state == ResourceState::Disk, "Trying to load a resource that is not on disk state");
	m_state = ResourceState::Pending;
	create_internal(_library, _device, _archive);
	m_state = ResourceState::Loaded;
}

void Resource::save(AssetLibrary* _library, aka::gfx::GraphicDevice* _device, Archive& _archive)
{
	AKA_ASSERT(m_state == ResourceState::Loaded, "Trying to save resource that is not loaded");
	save_internal(_library, _device, _archive);
}

void Resource::destroy(AssetLibrary* _library, aka::gfx::GraphicDevice* _device)
{
	AKA_ASSERT(m_state == ResourceState::Loaded, "Trying to destroy resource that is not loaded");
	m_state = ResourceState::Pending;
	destroy_internal(_library, _device);
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