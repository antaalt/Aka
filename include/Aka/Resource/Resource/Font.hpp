#pragma once

#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Resource/Archive/ArchiveFont.hpp>

namespace aka {

class Font : Resource
{
public:
	Font();
	Font(ResourceID _id, const String& _name);
	~Font();
private:
	void create_internal(AssetLibrary* library, Renderer* _renderer, const Archive& _archive) override;
	void save_internal(AssetLibrary* library, Renderer* _renderer, Archive& _archive) override;
	void destroy_internal(AssetLibrary* library, Renderer* _renderer) override;
};

}