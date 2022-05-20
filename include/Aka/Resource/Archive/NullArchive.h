#pragma once

#include <Aka/Resource/ResourceArchive.h>

namespace aka {

enum class NullArchiveVersion : uint32_t
{
	ArchiveCreation = 0,

	Latest = ArchiveCreation
};

struct NullArchive : ResourceArchive
{
	bool load(Stream& path, BuildData* data) override;
	bool save(Stream& path, const BuildData* data) override;
};

};