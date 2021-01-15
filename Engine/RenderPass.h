#pragma once

#include "Framebuffer.h"

namespace aka {

class RenderPass
{
	Framebuffer::Ptr framebuffer;


	void execute();
};

}