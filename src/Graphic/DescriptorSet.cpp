#include <Aka/Graphic/DescriptorSet.h>
#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

DescriptorSetData::DescriptorSetData() :
	count(0),
	buffers{},
	images{},
	samplers{}
{
}

DescriptorSetData& DescriptorSetData::addUniformBuffer(BufferHandle buffer)
{
	AKA_ASSERT(count + 1 < ShaderMaxBindingCount, "Too many shader bindings");
	buffers[count] = buffer;
	samplers[count] = gfx::SamplerHandle::null;
	images[count] = gfx::TextureHandle::null;
	count++;
	return *this;
}

DescriptorSetData& DescriptorSetData::addStorageBuffer(BufferHandle buffer)
{
	AKA_ASSERT(count + 1 < ShaderMaxBindingCount, "Too many shader bindings");
	buffers[count] = buffer;
	samplers[count] = gfx::SamplerHandle::null;
	images[count] = gfx::TextureHandle::null;
	count++;
	return *this;
}

DescriptorSetData& DescriptorSetData::addSampledImage(TextureHandle texture, SamplerHandle sampler)
{
	AKA_ASSERT(count + 1 < ShaderMaxBindingCount, "Too many shader bindings");
	images[count] = texture;
	samplers[count] = sampler;
	buffers[count] = gfx::BufferHandle::null;
	count++;
	return *this;
}

DescriptorSetData& DescriptorSetData::addStorageImage(TextureHandle texture)
{
	AKA_ASSERT(count + 1 < ShaderMaxBindingCount, "Too many shader bindings");
	images[count] = texture;
	samplers[count] = SamplerHandle::null;
	buffers[count] = gfx::BufferHandle::null;
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