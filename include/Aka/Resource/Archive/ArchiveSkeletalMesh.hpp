#pragma once

#include <Aka/Resource/Archive/Archive.hpp>
#include <Aka/Graphic/Pipeline.h>

namespace aka {

struct ArchiveSkeletalMesh : Archive
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,

		Latest = ArchiveCreation
	};
	ArchiveSkeletalMesh();
	explicit ArchiveSkeletalMesh(AssetID id);

	Vector<AssetID> animations;
	Vector<AssetID> batches;

protected:
	ArchiveParseResult parse(BinaryArchive& _archive) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

}