#pragma once

#include <Aka/Resource/ResourceArchive.h>

namespace aka {

enum class FontArchiveVersion : uint32_t
{
	ArchiveCreation = 0,

	Latest = ArchiveCreation
};

struct FontArchive : ResourceArchive
{
	bool load(Stream& path, BuildData* data) override;
	bool save(Stream& path, const BuildData* data) override;
};

};