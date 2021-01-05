#pragma once

#include <stdint.h>

namespace app {

class Framebuffer
{
public:
	Framebuffer() : m_width(0), m_height(0) {}

	virtual void create(uint32_t width, uint32_t height) = 0;
	virtual void destroy() = 0;

	virtual void use() = 0;

protected:
	uint32_t m_width;
	uint32_t m_height;
};

}