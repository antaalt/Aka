#pragma once

#include <Aka/Resource/Archive/Archive.hpp>

namespace aka {


// hold the ttf to be parsed or other depending on method(check valve paper with sdf).
struct ArchiveFont : Archive
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,

		Latest = ArchiveCreation
	};
	ArchiveFont();
	ArchiveFont(AssetID path);

	Blob ttf;

protected:
	ArchiveLoadResult load_internal(ArchiveLoadContext& _context, BinaryArchive& path) override;
	ArchiveSaveResult save_internal(ArchiveSaveContext& _context, BinaryArchive& path) override;
	ArchiveLoadResult load_dependency(ArchiveLoadContext& _context) override;
	ArchiveSaveResult save_dependency(ArchiveSaveContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };

	void copyFrom(const Archive* _archive) override;
};

}