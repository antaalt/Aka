#pragma once

#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/Sampler.h>
#include <Aka/Graphic/Resource.h>
#include <Aka/Graphic/DescriptorPool.h>

namespace aka {
namespace gfx {

struct DescriptorSlot
{
	ShaderBindingType bindingType;
	struct Texture {
		TextureHandle texture;
		SamplerHandle sampler; // Can be null.
		uint32_t layer;
		uint32_t mipLevel;
	};
	struct Buffer {
		BufferHandle handle;
		uint32_t offset;
		uint32_t range;
	};
	union
	{
		Texture texture;
		Buffer buffer;
	};
};

struct DescriptorSetData
{
	DescriptorSetData();

	DescriptorSetData& addUniformBuffer(BufferHandle buffer, uint32_t offset = 0, uint32_t range = ~0U);
	DescriptorSetData& addStorageBuffer(BufferHandle buffer, uint32_t offset = 0, uint32_t range = ~0U);
	DescriptorSetData& addSampledTexture2D(TextureHandle texture, SamplerHandle sampler, uint32_t layer = 0, uint32_t mipLevel = 0);
	//DescriptorSetData& addSampledTextureCube2D(TextureHandle texture, SamplerHandle sampler);
	DescriptorSetData& addStorageTexture2D(TextureHandle texture, uint32_t layer = 0, uint32_t mipLevel = 0);

	uint32_t count;
	DescriptorSlot slots[ShaderMaxBindingCount];
};

struct DescriptorSet;
using DescriptorSetHandle = ResourceHandle<DescriptorSet>;

struct DescriptorSet : Resource
{
	DescriptorSet(const char* name, const ShaderBindingState& bindings, DescriptorPoolHandle pool);

	DescriptorPoolHandle pool;
	ShaderBindingState bindings;

	void update(const DescriptorSetData& data);

	static DescriptorSetHandle allocate(const char* name, const ShaderBindingState& state, DescriptorPoolHandle pool);
	static void free(DescriptorSetHandle set);
};

};
};