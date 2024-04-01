#pragma once

#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Resource/Archive/ArchiveFont.hpp>

namespace aka {

class Font : public Resource
{
public:
	Font();
	Font(AssetID _id, const String& _name);
	~Font();
private:
	void fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer) override;
	void toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer) override;
	void destroy_internal(AssetLibrary* library, Renderer* _renderer) override;
};

}