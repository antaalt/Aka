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
	explicit ArchiveFont(AssetID path);

	Blob ttf;

protected:
	ArchiveParseResult parse(BinaryArchive& path) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

}