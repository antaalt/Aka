#include "Time.h"

namespace app {


const std::chrono::time_point<std::chrono::steady_clock> g_startup = std::chrono::steady_clock::now();

uint64_t Time::now()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - g_startup).count();
}

}