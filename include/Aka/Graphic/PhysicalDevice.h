#pragma once

#include <stdint.h>

namespace aka {
namespace gfx {

struct PhysicalDevice
{
	const char* name;
	uint32_t getDeviceProperties();
	uint32_t getDeviceFeatures();
	uint32_t getDeviceLimits();
	// properties...
};

};
};