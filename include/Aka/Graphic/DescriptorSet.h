#pragma once

#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/Sampler.h>
#include <Aka/Graphic/Resource.h>

namespace aka {
namespace gfx {

struct DescriptorSetData
{
	DescriptorSetData();

	DescriptorSetData& addUniformBuffer(BufferHandle buffer);
	DescriptorSetData& addStorageBuffer(BufferHandle buffer);
	DescriptorSetData& addSampledImage(TextureHandle texture, SamplerHandle sampler);
	DescriptorSetData& addStorageImage(TextureHandle texture);

	uint32_t count;
	BufferHandle buffers[ShaderMaxBindingCount];
	TextureHandle images[ShaderMaxBindingCount];
	SamplerHandle samplers[ShaderMaxBindingCount];
};

struct DescriptorSet;
using DescriptorSetHandle = ResourceHandle<DescriptorSet>;

struct DescriptorSet : Resource
{
	DescriptorSet(const char* name, const ShaderBindingState& bindings);

	ShaderBindingState bindings;

	void update(const DescriptorSetData& data);

	static DescriptorSetHandle create(const char* name, const ShaderBindingState& state);
	static void destroy(DescriptorSetHandle set);
};

};
};