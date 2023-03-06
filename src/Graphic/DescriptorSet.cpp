#include <Aka/Graphic/DescriptorSet.h>
#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

void DescriptorSetData::setUniformBuffer(uint32_t slot, BufferHandle buffer)
{
	/*AKA_ASSERT(slot < bindings.count, "Invalid slot.");
	if (bindings.bindings[slot].type != ShaderBindingType::UniformBuffer)
		return; */
	buffers[slot] = buffer;
}

void DescriptorSetData::setStorageBuffer(uint32_t slot, BufferHandle buffer)
{
	/*AKA_ASSERT(slot < bindings.count, "Invalid slot.");
	if (bindings.bindings[slot].type != ShaderBindingType::UniformBuffer)
		return; */
	buffers[slot] = buffer;
}

void DescriptorSetData::setSampledImage(uint32_t slot, TextureHandle texture, SamplerHandle sampler)
{
	/*AKA_ASSERT(slot < bindings.count, "Invalid slot.");
	if (bindings.bindings[slot].type != ShaderBindingType::SampledImage)
		return; */
	images[slot] = texture;
	samplers[slot] = sampler;
}

void DescriptorSetData::setStorageImage(uint32_t slot, TextureHandle texture)
{
	/*AKA_ASSERT(slot < bindings.count, "Invalid slot.");
	if (bindings.bindings[slot].type != ShaderBindingType::StorageImage)
		return;*/
	images[slot] = texture;
	samplers[slot] = SamplerHandle::null;
}

DescriptorSet::DescriptorSet(const char* name, const ShaderBindingState& bindings) :
	Resource(name, ResourceType::DescriptorSet),
	bindings(bindings)
{
}

DescriptorSetHandle DescriptorSet::create(const char* name, const ShaderBindingState& bindings)
{
	return Application::app()->graphic()->createDescriptorSet(name, bindings);
}

void DescriptorSet::update(const DescriptorSetData& data)
{
	Application::app()->graphic()->update(DescriptorSetHandle{ this }, data);
}

void DescriptorSet::destroy(DescriptorSetHandle descriptorSet)
{
	Application::app()->graphic()->destroy(descriptorSet);
}

};
};