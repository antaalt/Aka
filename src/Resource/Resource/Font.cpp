#include <Aka/Resource/Resource/Font.hpp>

namespace aka {

Font::Font() :
	Resource(ResourceType::First, ResourceID::Invalid, "") 
{
}
Font::Font(ResourceID id, const aka::String& _name) :
	Resource(ResourceType::First, id, name)
{
}

void Font::create(AssetLibrary* _library, aka::gfx::GraphicDevice* _device, const Archive& _archive)
{
	AKA_ASSERT(m_state == ResourceState::Disk, "Trying to load a resource that is not on disk state");
	m_state = ResourceState::Pending;
	create_internal(_library, _device, _archive);
	m_state = ResourceState::Loaded;
}

void Font::save(AssetLibrary* _library, aka::gfx::GraphicDevice* _device, Archive& _archive)
{
	AKA_ASSERT(m_state == ResourceState::Loaded, "Trying to save resource that is not loaded");
	save_internal(_library, _device, _archive);
}

void Font::destroy(AssetLibrary* _library, aka::gfx::GraphicDevice* _device)
{
	AKA_ASSERT(m_state == ResourceState::Loaded, "Trying to destroy resource that is not loaded");
	m_state = ResourceState::Pending;
	destroy_internal(_library, _device);
	m_state = ResourceState::Disk;
}

}