#pragma once 

#include <Aka/Resource/Archive/ArchiveImage.hpp>

namespace aka {

struct ArchiveSpriteFrame 
{
	ArchiveImage image; // Should use single texture atlas.
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
	ArchiveSprite(AssetID id);

	uint32_t width;
	uint32_t height;
	Vector<ArchiveSpriteAnimation> animations;

	ArchiveSpriteAnimation& find(const char* name);

protected:
	ArchiveLoadResult load_internal(ArchiveLoadContext& _context, BinaryArchive& path) override;
	ArchiveSaveResult save_internal(ArchiveSaveContext& _context, BinaryArchive& path) override;
	ArchiveLoadResult load_dependency(ArchiveLoadContext& _context) override;
	ArchiveSaveResult save_dependency(ArchiveSaveContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };

	void copyFrom(const Archive* _archive) override;
};

};