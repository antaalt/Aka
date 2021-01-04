#pragma once

#include <vector>

namespace app {

struct Image
{
	static Image load(const char* name);

	std::vector<uint8_t> bytes;
	uint32_t width, height;
};

}