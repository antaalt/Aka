#pragma once

#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/Sampler.h>
#include <Aka/Graphic/Resource.h>
#include <Aka/Graphic/DescriptorPool.h>

namespace aka {
namespace gfx {

struct DescriptorSlotTexture
{
	TextureHandle texture;
	SamplerHandle sampler; // Can be null.
	uint32_t layer;
	uint32_t mipLevel;
};
struct DescriptorSlotBuffer
{
	BufferHandle handle;
	uint32_t offset;
	uint32_t range;
};
struct DescriptorUpdate
{
	ShaderBindingType bindingType;
	uint32_t binding;
	uint32_t index;
	union
	{
		DescriptorSlotTexture texture;
		DescriptorSlotBuffer buffer;
	};
	static DescriptorUpdate sampledTexture2D(uint32_t binding, uint32_t index, TextureHandle texture, SamplerHandle sampler, uint32_t layer = 0, uint32_t mipLevel = 0);
	static DescriptorUpdate sampledTextureCube2D(uint32_t binding, uint32_t index, TextureHandle texture, SamplerHandle sampler, uint32_t mipLevel = 0);
	static DescriptorUpdate storageTexture2D(uint32_t binding, uint32_t index, TextureHandle texture, uint32_t layer = 0, uint32_t mipLevel = 0);
	static DescriptorUpdate uniformBuffer(uint32_t binding, uint32_t index, BufferHandle buffer, uint32_t offset = 0, uint32_t range = ~0);
	static DescriptorUpdate storageBuffer(uint32_t binding, uint32_t index, BufferHandle buffer, uint32_t offset = 0, uint32_t range = ~0);
	static DescriptorUpdate structuredBuffer(uint32_t binding, uint32_t index, BufferHandle buffer, uint32_t offset = 0, uint32_t range = ~0);
};

struct DescriptorSet;
using DescriptorSetHandle = ResourceHandle<DescriptorSet>;

struct DescriptorSet : Resource
{
	DescriptorSet(const char* name, const ShaderBindingState& bindings, DescriptorPoolHandle pool);

	DescriptorPoolHandle pool;
	ShaderBindingState bindings;

	static void update(DescriptorSetHandle descriptorSet, const DescriptorUpdate* update, size_t size);

	static DescriptorSetHandle allocate(const char* name, const ShaderBindingState& state, DescriptorPoolHandle pool);
	static void free(DescriptorSetHandle set);
};

};
};