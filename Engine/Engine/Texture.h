#pragma once

#include <stdint.h>

namespace app {

class Texture
{
public:
	virtual void create(uint32_t width, uint32_t height, const void* data) = 0;
	virtual void destroy() = 0;
private:
	uint32_t width;
	uint32_t height;
};

}
