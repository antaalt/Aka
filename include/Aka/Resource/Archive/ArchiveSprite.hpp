#pragma once 

#include <Aka/Resource/Archive/Archive.hpp>

namespace aka {

struct ArchiveSpriteFrame 
{
	AssetID image; // Should use single texture atlas.
	Time duration;
};
struct ArchiveSpriteAnimation 
{
	String name;
	Vector<ArchiveSpriteFrame> frames;
};

struct ArchiveSprite : Archive
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,

		Latest = ArchiveCreation
	};
	ArchiveSprite();
	explicit ArchiveSprite(AssetID id);

	uint32_t width;
	uint32_t height;
	Vector<ArchiveSpriteAnimation> animations;

	ArchiveSpriteAnimation& find(const char* name);

protected:
	ArchiveParseResult parse(BinaryArchive& path) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

};