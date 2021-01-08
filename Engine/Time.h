#pragma once

#include <stdint.h>
#include <chrono>

namespace app {

struct Time
{
	using unit = uint64_t;
	// Time elapsed since app startup
	static uint64_t now();
};

};
