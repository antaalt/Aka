#pragma once 

#include <Aka/Resource/Archive/Archive.hpp>

namespace aka {

struct ArchiveSkeletalBone
{
	uint32_t parentIndex;
	String name;
	mat4f offset;
};

struct ArchiveSkeleton : Archive
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,

		Latest = ArchiveCreation
	};
	ArchiveSkeleton();
	explicit ArchiveSkeleton(AssetID id);

	uint32_t rootBoneIndex;
	Vector<ArchiveSkeletalBone> bones;
protected:
	ArchiveParseResult parse(BinaryArchive& path) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

};