#pragma once

#include <vector>
#include "FileSystem.h"

namespace aka {

struct Image
{
	static Image load(const Path& path);
	static Image load(const uint8_t* binaries, size_t size);
	static Image load(const std::vector<uint8_t>& binaries);

	std::vector<uint8_t> bytes;
	uint32_t width, height;
};

}