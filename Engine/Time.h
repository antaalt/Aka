#pragma once

#include <stdint.h>
#include <chrono>

namespace app {

struct Time
{
	using unit = uint64_t;
	static uint64_t now();
};

};
