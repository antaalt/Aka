#include <Aka/Graphic/DescriptorSet.h>
#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

DescriptorSetData::DescriptorSetData() :
	count(0),
	slots{}
{
}

DescriptorSetData& DescriptorSetData::addUniformBuffer(BufferHandle buffer, uint32_t offset, uint32_t range)
{
	AKA_ASSERT(count + 1 < ShaderMaxBindingCount, "Too many shader bindings");
	slots[count].bindingType = ShaderBindingType::UniformBuffer;
	slots[count].buffer.handle = buffer;
	slots[count].buffer.offset = offset;
	slots[count].buffer.range = range;
	count++;
	return *this;
}

DescriptorSetData& DescriptorSetData::addStorageBuffer(BufferHandle buffer, uint32_t offset, uint32_t range)
{
	AKA_ASSERT(count + 1 < ShaderMaxBindingCount, "Too many shader bindings");
	slots[count].bindingType = ShaderBindingType::StorageBuffer;
	slots[count].buffer.handle = buffer;
	slots[count].buffer.offset = offset;
	slots[count].buffer.range = range;
	count++;
	return *this;
}

DescriptorSetData& DescriptorSetData::addSampledTexture2D(TextureHandle texture, SamplerHandle sampler, uint32_t layer, uint32_t mipLevel)
{
	AKA_ASSERT(count + 1 < ShaderMaxBindingCount, "Too many shader bindings");
	slots[count].bindingType = ShaderBindingType::SampledImage;
	slots[count].texture.texture = texture;
	slots[count].texture.sampler = sampler;
	slots[count].texture.layer = layer;
	slots[count].texture.mipLevel = mipLevel;
	count++;
	return *this;
}

DescriptorSetData& DescriptorSetData::addStorageTexture2D(TextureHandle texture, uint32_t layer, uint32_t mipLevel)
{
	AKA_ASSERT(count + 1 < ShaderMaxBindingCount, "Too many shader bindings");
	slots[count].bindingType = ShaderBindingType::StorageImage;
	slots[count].texture.texture = texture;
	slots[count].texture.sampler = gfx::SamplerHandle::null;
	slots[count].texture.layer = layer;
	slots[count].texture.mipLevel = mipLevel;
	count++;
	return *this;
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