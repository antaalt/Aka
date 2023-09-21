#pragma once

#include <Aka/Resource/Archive/ArchiveImage.hpp>
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
	ArchiveMaterial(const AssetID& id);

	ArchiveMaterialFlag flags;
	color4f color;
	ArchiveImage albedo;
	ArchiveImage normal;

protected:
	ArchiveLoadResult load_internal(ArchiveLoadContext& _context, BinaryArchive& path) override;
	ArchiveSaveResult save_internal(ArchiveSaveContext& _context, BinaryArchive& path) override;
	ArchiveLoadResult load_dependency(ArchiveLoadContext& _context) override;
	ArchiveSaveResult save_dependency(ArchiveSaveContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };

	void copyFrom(const Archive* _archive) override;
};

}