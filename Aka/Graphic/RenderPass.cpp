#include "RenderPass.h"
#include "GraphicBackend.h"
#include "../OS/Logger.h"

namespace aka {

void RenderPass::execute()
{
	if (this->shader == nullptr)
	{
		Logger::error("No shader set for render pass.");
		return;
	}
	if (this->texture == nullptr)
	{
		uint8_t data[4] = { 255, 255, 255, 255 };
		this->texture = Texture::create(1, 1, Texture::Format::Rgba, data, Sampler::Filter::Nearest);
		Logger::warn("No texture set for render pass.");
	}
	if (this->mesh == nullptr)
	{
		this->mesh = Mesh::create();
		Logger::warn("No mesh set for render pass.");
	}
	GraphicBackend::render(*this);
}

};