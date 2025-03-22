#pragma once

#include <Aka/Resource/Archive/Archive.hpp>	

namespace aka {

enum class ArchiveImageFormat {
	Uncompressed, // Raw bytes
	Compressed, // Standard image format (JPG / DDS...)
	Bc1, // Raw Bc1
	Bc2, // Raw Bc2
	Bc3, // Raw Bc3
	Bc4, // Raw Bc4
	Bc5, // Raw Bc5
};

struct ArchiveImage : Archive
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,

		Latest = ArchiveCreation
	};
	ArchiveImage();
	explicit ArchiveImage(AssetID id);

	size_t size() const { return data.size(); }

	ArchiveImageFormat format;
	Vector<uint8_t> data;
	uint32_t width;
	uint32_t height;
	uint32_t channels;

protected:
	ArchiveParseResult parse(BinaryArchive& path) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

}