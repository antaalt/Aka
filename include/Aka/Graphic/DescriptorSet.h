#pragma once

#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/Sampler.h>

namespace aka {

struct DescriptorSet
{
	ShaderBindingState bindings;

	Buffer* buffers[ShaderBindingState::MaxBindingCount];
	Texture* images[ShaderBindingState::MaxBindingCount];
	Sampler* samplers[ShaderBindingState::MaxBindingCount];


	void setUniformBuffer(uint32_t slot, Buffer* buffer);
	void setStorageBuffer(uint32_t slot, Buffer* buffer);
	void setSampledImage(uint32_t slot, Texture* texture, Sampler* sampler);
	void setStorageImage(uint32_t slot, Texture* texture);

	static DescriptorSet* create(const ShaderBindingState& state);
	static void destroy(DescriptorSet* set);
};

};