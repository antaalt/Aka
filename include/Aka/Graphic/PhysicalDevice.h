#pragma once

#include <stdint.h>

namespace aka {

struct PhysicalDevice
{
	void* native;

	const char* name;
	uint32_t getDeviceProperties();
	uint32_t getDeviceFeatures();
	uint32_t getDeviceLimits();
	// properties...
};

};