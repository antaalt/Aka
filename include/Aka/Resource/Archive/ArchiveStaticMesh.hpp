#pragma once

#include <Aka/Resource/Archive/Archive.hpp>

namespace aka {

struct ArchiveStaticMesh : Archive 
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,

		Latest = ArchiveCreation
	};
	ArchiveStaticMesh();
	explicit ArchiveStaticMesh(AssetID id);

	// LOD will be generated at runtime, maybe cache it somewhere for perf...
	// Use simplygon or mesh opt...
	Vector<AssetID> batches;

protected:
	ArchiveParseResult parse(BinaryArchive& _archive) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

}