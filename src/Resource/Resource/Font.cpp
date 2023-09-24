#include <Aka/Resource/Resource/Font.hpp>

namespace aka {

Font::Font() :
	Resource(ResourceType::Font, AssetID::Invalid, "")
{
}
Font::Font(AssetID id, const aka::String& _name) :
	Resource(ResourceType::Font, id, _name)
{
}

void Font::create_internal(AssetLibrary* _library, Renderer* _renderer, const Archive& _archive)
{
	AKA_NOT_IMPLEMENTED;
}

void Font::save_internal(AssetLibrary* _library, Renderer* _renderer, Archive& _archive)
{
	AKA_NOT_IMPLEMENTED;
}

void Font::destroy_internal(AssetLibrary* _library, Renderer* _renderer)
{
	AKA_NOT_IMPLEMENTED;
}

}