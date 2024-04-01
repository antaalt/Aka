#pragma once

#include <Aka/Resource/Archive/Archive.hpp>
#include <Aka/Graphic/Pipeline.h>

namespace aka {

struct ArchiveBatch : Archive
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,

		Latest = ArchiveCreation
	};
	ArchiveBatch();
	explicit ArchiveBatch(AssetID path);

	AssetID material;
	AssetID geometry;

protected:
	ArchiveParseResult parse(BinaryArchive& path) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

}