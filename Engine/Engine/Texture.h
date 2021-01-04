#pragma once

#include <stdint.h>

namespace app {

class Texture
{
public:
	using ID = uint32_t;
public:
	virtual void create(uint32_t width, uint32_t height, const void* data) = 0;
	virtual void destroy() = 0;

	virtual ID getID() = 0;

private:
	uint32_t width;
	uint32_t height;
};

}
