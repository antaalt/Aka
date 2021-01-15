#include "RenderPass.h"


namespace aka {

void RenderPass::execute()
{
	framebuffer->bind(Framebuffer::Type::Both);

	// Enable depth, transparency, cull & everything depending on settings.

}

};