#pragma once

#include <Aka/Resource/ResourceArchive.h>

namespace aka {

enum class SpriteArchiveVersion : uint32_t
{
	ArchiveCreation = 0,

	Latest = ArchiveCreation
};

struct SpriteArchive : ResourceArchive
{
	bool load(Stream& path, BuildData* data) override;
	bool save(Stream& path, const BuildData* data) override;
};

};