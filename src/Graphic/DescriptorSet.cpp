#include <Aka/Graphic/DescriptorSet.h>
#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

DescriptorUpdate DescriptorUpdate::sampledTexture2D(uint32_t binding, uint32_t index, TextureHandle texture, SamplerHandle sampler, uint32_t layer, uint32_t mipLevel)
{
	DescriptorUpdate update{};
	update.bindingType = ShaderBindingType::SampledImage;
	update.binding = binding;
	update.index = index;
	update.texture.texture = texture;
	update.texture.sampler = sampler;
	update.texture.layer = layer;
	update.texture.mipLevel = mipLevel;
	return update;
}
DescriptorUpdate DescriptorUpdate::sampledTextureCube2D(uint32_t binding, uint32_t index, TextureHandle texture, SamplerHandle sampler, uint32_t mipLevel)
{
	DescriptorUpdate update{};
	update.bindingType = ShaderBindingType::SampledImage;
	update.binding = binding;
	update.index = index;
	update.texture.texture = texture;
	update.texture.sampler = sampler;
	update.texture.layer;
	update.texture.mipLevel = mipLevel;
	return update;
}
DescriptorUpdate DescriptorUpdate::storageTexture2D(uint32_t binding, uint32_t index, TextureHandle texture, uint32_t layer, uint32_t mipLevel)
{
	DescriptorUpdate update{};
	update.bindingType = ShaderBindingType::StorageImage;
	update.binding = binding;
	update.index = index;
	update.texture.texture = texture;
	update.texture.sampler = gfx::SamplerHandle::null;
	update.texture.layer = layer;
	update.texture.mipLevel = mipLevel;
	return update;
}
DescriptorUpdate DescriptorUpdate::uniformBuffer(uint32_t binding, uint32_t index, BufferHandle buffer, uint32_t offset, uint32_t range)
{
	DescriptorUpdate update{};
	update.bindingType = ShaderBindingType::UniformBuffer;
	update.binding = binding;
	update.index = index;
	update.buffer.handle = buffer;
	update.buffer.offset = offset;
	update.buffer.range = range;
	return update;
}
DescriptorUpdate DescriptorUpdate::storageBuffer(uint32_t binding, uint32_t index, BufferHandle buffer, uint32_t offset, uint32_t range)
{
	DescriptorUpdate update{};
	update.bindingType = ShaderBindingType::StorageBuffer;
	update.binding = binding;
	update.index = index;
	update.buffer.handle = buffer;
	update.buffer.offset = offset;
	update.buffer.range = range;
	return update;
}
DescriptorUpdate DescriptorUpdate::structuredBuffer(uint32_t binding, uint32_t index, BufferHandle buffer, uint32_t offset, uint32_t range)
{
	DescriptorUpdate update{};
	update.bindingType = ShaderBindingType::None;
	update.binding = binding;
	update.index = index;
	update.buffer.handle = buffer;
	update.buffer.offset = offset;
	update.buffer.range = range;
	return update;
}

DescriptorSet::DescriptorSet(const char* name, const ShaderBindingState& bindings, DescriptorPoolHandle pool) :
	Resource(name, ResourceType::DescriptorSet),
	pool(pool),
	bindings(bindings)
{
}

};
};