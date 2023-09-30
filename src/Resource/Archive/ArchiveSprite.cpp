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
ArchiveParseResult ArchiveSprite::parse(BinaryArchive& _archive)
{
	AKA_NOT_IMPLEMENTED;
	return ArchiveParseResult::Success;
}

ArchiveParseResult ArchiveSprite::load_dependency(ArchiveLoadContext& _context)
{
	AKA_NOT_IMPLEMENTED;
	return ArchiveParseResult::Success;
}

};