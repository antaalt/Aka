#include <Aka/Resource/Archive/ArchiveSprite.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveSpriteAnimation& ArchiveSprite::find(const char* name)
{
	for (auto& animation : animations)
	{
		if (animation.name == String(name))
		{
			return animation;
		}
	}
	return animations[0]; // default if not found
}

ArchiveSprite::ArchiveSprite() :
	ArchiveSprite(AssetID::Invalid)
{
}
ArchiveSprite::ArchiveSprite(AssetID id) :
	Archive(AssetType::Sprite, id)
{
}
ArchiveLoadResult ArchiveSprite::load_internal(ArchiveLoadContext& _context, BinaryArchive& _archive)
{
	AKA_NOT_IMPLEMENTED;
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveSprite::save_internal(ArchiveSaveContext& _context, BinaryArchive& _archive)
{
	AKA_NOT_IMPLEMENTED;
	return ArchiveSaveResult::Success;
}

ArchiveLoadResult ArchiveSprite::load_dependency(ArchiveLoadContext& _context)
{
	AKA_NOT_IMPLEMENTED;
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveSprite::save_dependency(ArchiveSaveContext& _context)
{
	AKA_NOT_IMPLEMENTED;
	return ArchiveSaveResult::Success;
}
void ArchiveSprite::copyFrom(const Archive* _archive)
{
	AKA_ASSERT(_archive->id() == id(), "Invalid id");
	AKA_ASSERT(_archive->type() == type(), "Invalid type");
	AKA_ASSERT(_archive->version() == version(), "Invalid version");

	const ArchiveSprite* archive = reinterpret_cast<const ArchiveSprite*>(_archive);
	*this = *archive;
}

};