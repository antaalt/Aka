#pragma once

#include <Aka/Resource/Archive/ArchiveMaterial.hpp>	
#include <Aka/Resource/Archive/ArchiveGeometry.hpp>

namespace aka {

struct ArchiveBatch : Archive
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,

		Latest = ArchiveCreation
	};
	ArchiveBatch();
	ArchiveBatch(AssetID path);

	ArchiveMaterial material;
	ArchiveGeometry geometry;

protected:
	ArchiveLoadResult load_internal(ArchiveLoadContext& _context, BinaryArchive& path) override;
	ArchiveSaveResult save_internal(ArchiveSaveContext& _context, BinaryArchive& path) override;
	ArchiveLoadResult load_dependency(ArchiveLoadContext& _context) override;
	ArchiveSaveResult save_dependency(ArchiveSaveContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };

	void copyFrom(const Archive* _archive) override;
};

}