#pragma once

#include <Aka/Resource/Archive/Archive.hpp>
#include <Aka/Core/Enum.h>

namespace aka {

enum class ArchiveMaterialFlag : uint32_t
{
	None		= 0,
	DoubleSided = 1 << 0,
	AlphaTest	= 1 << 1,
};
AKA_IMPLEMENT_BITMASK_OPERATOR(ArchiveMaterialFlag);


struct ArchiveMaterial : Archive
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,

		Latest = ArchiveCreation
	};

	ArchiveMaterial();
	explicit ArchiveMaterial(AssetID id);

	ArchiveMaterialFlag flags;
	color4f color;
	AssetID albedo;
	AssetID normal;

protected:
	ArchiveParseResult parse(BinaryArchive& path) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

}