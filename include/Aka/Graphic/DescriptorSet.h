#pragma once

#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/Sampler.h>
#include <Aka/Graphic/Resource.h>

namespace aka {
namespace gfx {

struct DescriptorSet;
using DescriptorSetHandle = ResourceHandle<DescriptorSet>;

struct DescriptorSetData
{
	union {
		const Buffer* buffers[ShaderBindingState::MaxBindingCount];
		struct {
			TextureHandle images[ShaderBindingState::MaxBindingCount];
			const Sampler* samplers[ShaderBindingState::MaxBindingCount];
		};
	};

	void setUniformBuffer(uint32_t slot, const Buffer* buffer);
	void setStorageBuffer(uint32_t slot, const Buffer* buffer);
	void setSampledImage(uint32_t slot, TextureHandle texture, const Sampler* sampler);
	void setStorageImage(uint32_t slot, TextureHandle texture);
};

struct DescriptorSet;
using DescriptorSetHandle = ResourceHandle<DescriptorSet>;

struct DescriptorSet : Resource
{
	ShaderBindingState bindings;

	void update(const DescriptorSetData& data);

	static DescriptorSetHandle create(const ShaderBindingState& state);
	static void destroy(DescriptorSetHandle set);
};

};
};