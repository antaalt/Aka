#include <Aka/Rendering/Material.h>
#include <Aka/Core/Application.h>

namespace aka {

void Material::setUniformBuffer(uint32_t slot, Buffer* buffer)
{
	AKA_ASSERT(slot < program->bindings.count, "Invalid slot.");
	// TODO get slot ID from layout
	if (program->bindings.bindings[slot].type != ShaderBindingType::UniformBuffer)
		return;
	buffers[slot] = buffer;
}

void Material::setStorageBuffer(uint32_t slot, Buffer* buffer)
{
	AKA_ASSERT(slot < program->bindings.count, "Invalid slot.");
	if (program->bindings.bindings[slot].type != ShaderBindingType::UniformBuffer)
		return;
	buffers[slot] = buffer;
}

void Material::setSampledImage(uint32_t slot, Texture* texture, Sampler* sampler)
{
	AKA_ASSERT(slot < program->bindings.count, "Invalid slot.");
	if (program->bindings.bindings[slot].type != ShaderBindingType::SampledImage)
		return;
	images[slot] = texture;
	samplers[slot] = sampler;
}

void Material::setStorageImage(uint32_t slot, Texture* texture)
{
	AKA_ASSERT(slot < program->bindings.count, "Invalid slot.");
	if (program->bindings.bindings[slot].type != ShaderBindingType::StorageImage)
		return;
	images[slot] = texture;
	samplers[slot] = nullptr;
}

Material* Material::create(Program* program)
{
	return Application::app()->graphic()->createMaterial(program);
}

void Material::destroy(Material* material)
{
	Application::app()->graphic()->destroy(material);
}

};