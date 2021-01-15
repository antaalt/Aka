#pragma once

#include "Framebuffer.h"
#include "Shader.h"

namespace aka {

struct RenderPass
{
	Framebuffer* framebuffer;

	Mesh* mesh;

	Shader *shader;

	void execute();
};

}