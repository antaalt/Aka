#pragma once

#include "Framebuffer.h"

namespace aka {

class Backbuffer : public Framebuffer
{
public:
	using Ptr = std::shared_ptr<Backbuffer>;
	friend class GraphicBackend;

	Backbuffer();

	static Backbuffer::Ptr create();

private:
	void setSize(uint32_t width, uint32_t height);
};

}

