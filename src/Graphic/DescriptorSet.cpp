#include <Aka/Graphic/DescriptorSet.h>
#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

void DescriptorSet::setUniformBuffer(uint32_t slot, Buffer* buffer)
{
	AKA_ASSERT(slot < bindings.count, "Invalid slot.");
	if (bindings.bindings[slot].type != ShaderBindingType::UniformBuffer)
		return;
	buffers[slot] = buffer;
}

void DescriptorSet::setStorageBuffer(uint32_t slot, Buffer* buffer)
{
	AKA_ASSERT(slot < bindings.count, "Invalid slot.");
	if (bindings.bindings[slot].type != ShaderBindingType::UniformBuffer)
		return;
	buffers[slot] = buffer;
}

void DescriptorSet::setSampledImage(uint32_t slot, Texture* texture, Sampler* sampler)
{
	AKA_ASSERT(slot < bindings.count, "Invalid slot.");
	if (bindings.bindings[slot].type != ShaderBindingType::SampledImage)
		return;
	images[slot] = texture;
	samplers[slot] = sampler;
}

void DescriptorSet::setStorageImage(uint32_t slot, Texture* texture)
{
	AKA_ASSERT(slot < bindings.count, "Invalid slot.");
	if (bindings.bindings[slot].type != ShaderBindingType::StorageImage)
		return;
	images[slot] = texture;
	samplers[slot] = nullptr;
}

DescriptorSet* DescriptorSet::create(const ShaderBindingState& bindings)
{
	return Application::app()->graphic()->createDescriptorSet(bindings);
}

void DescriptorSet::destroy(DescriptorSet* descriptorSet)
{
	Application::app()->graphic()->destroy(descriptorSet);
}

};
};