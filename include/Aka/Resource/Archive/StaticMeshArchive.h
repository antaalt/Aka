#pragma once

#include <Aka/Resource/ResourceArchive.h>
#include <Aka/Graphic/Buffer.h>

namespace aka {

enum class StaticMeshArchiveVersion : uint32_t
{
	ArchiveCreation = 0,

	Latest = ArchiveCreation
};

struct StaticMeshArchive : ResourceArchive
{
	bool load(Stream& path, BuildData* data) override;
	bool save(Stream& path, const BuildData* data) override;
};

};