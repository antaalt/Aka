#pragma once

#include <Aka/Resource/Archive/ArchiveBatch.hpp>

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

	Vector<AssetID> batches;

protected:
	ArchiveParseResult parse(BinaryArchive& _archive) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

}