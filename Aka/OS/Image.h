#pragma once

#include <vector>
#include "FileSystem.h"

namespace aka {

struct Image
{
	static Image load(const Path& path);
	static Image load(const uint8_t* binaries, size_t size);
	static Image load(const std::vector<uint8_t>& binaries);

	static Image create(uint32_t width, uint32_t height, uint32_t components);
	static Image create(uint32_t width, uint32_t height, uint32_t components, const uint8_t *data);

	void save(const Path& path);

	std::vector<uint8_t> bytes;
	uint32_t width, height;
};

}