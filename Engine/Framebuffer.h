#pragma once

#include <stdint.h>

#include "Texture.h"

namespace app {

using FramebufferID = uint32_t;

enum class FramebufferType {
	READ,
	DRAW,
	BOTH
};

class Framebuffer
{
public:
	Framebuffer();
	~Framebuffer();

	void create(uint32_t width, uint32_t height, Texture* colorAttachment);
	void destroy();

	// Bind the framebuffer
	void bind(FramebufferType type);
	static void bindDefault(FramebufferType type);

	FramebufferID getID();

protected:
	uint32_t m_width;
	uint32_t m_height;
	FramebufferID m_framebufferID;
};

}