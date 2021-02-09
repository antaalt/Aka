#pragma once

#include <stdint.h>
#include <memory>
#include <vector>

namespace aka {

struct Monitor
{
	uint32_t width;
	uint32_t height;
	uint32_t numerator;
	uint32_t denominator;
};

struct Device
{
	static Device getDefault();
	static Device get(uint32_t id);
	static uint32_t count();

	char renderer[128]; // Renderer of the device
	char vendor[128]; // Vendor of the device
	char version[128]; // Version of the device
	uint32_t memory; // dedicated memory
	std::vector<Monitor> monitors; // supported monitors
};

}