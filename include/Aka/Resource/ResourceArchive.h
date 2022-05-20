#pragma once

#include <Aka/OS/Stream/Stream.h>
#include <Aka/Resource/Resource.h>

#include <functional>

namespace aka {

struct BuildData;

// An archive is responsible of reading the resource from archive file and saving it
struct ResourceArchive
{
	// Load the resource from archive file
	virtual bool load(Stream& path, BuildData* data) = 0;
	// Save the resource to archive file
	virtual bool save(Stream& path, const BuildData* data) = 0;
};

};