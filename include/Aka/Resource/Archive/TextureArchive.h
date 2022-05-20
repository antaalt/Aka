#pragma once

#include <Aka/Resource/ResourceArchive.h>
#include <Aka/OS/Image.h>
#include <Aka/Graphic/Texture.h>

namespace aka {

enum class TextureArchiveVersion : uint32_t
{
	ArchiveCreation = 0,

	Latest = ArchiveCreation
};

struct TextureArchive : ResourceArchive
{
	bool load(Stream& path, BuildData* data) override;
	bool save(Stream& path, const BuildData* data) override;
};

};