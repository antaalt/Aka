#pragma once

#include <Aka/Resource/Archive/ArchiveBatch.hpp>

namespace aka {

struct ArchiveStaticMesh : Archive 
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,

		Latest = ArchiveCreation
	};
	ArchiveStaticMesh();
	ArchiveStaticMesh(AssetID id);

	// LOD will be generated at runtime, maybe cache it somewhere for perf...
	// Use simplygon or mesh opt...
	Vector<ArchiveBatch> batches;

protected:
	ArchiveLoadResult load_internal(ArchiveLoadContext& _context, BinaryArchive& path) override;
	ArchiveSaveResult save_internal(ArchiveSaveContext& _context, BinaryArchive& path) override;
	ArchiveLoadResult load_dependency(ArchiveLoadContext& _context) override;
	ArchiveSaveResult save_dependency(ArchiveSaveContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };

	void copyFrom(const Archive* _archive) override;
};

}