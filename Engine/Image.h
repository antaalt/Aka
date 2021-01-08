#pragma once

#include <vector>
#include "System.h"

namespace app {

struct Image
{
	static Image load(const Path& path);

	std::vector<uint8_t> bytes;
	uint32_t width, height;
};

}