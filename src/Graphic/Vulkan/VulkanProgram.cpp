#include "VulkanProgram.h"

#include "VulkanGraphicDevice.h"

namespace aka {

VkDescriptorType tovk(ShaderBindingType type)
{
	switch (type)
	{
	default:
	case aka::ShaderBindingType::SampledImage:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	case aka::ShaderBindingType::StorageImage:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case aka::ShaderBindingType::UniformBuffer:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case aka::ShaderBindingType::StorageBuffer:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case aka::ShaderBindingType::AccelerationStructure:
		return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	}
}

VkShaderStageFlags tovk(ShaderType shaderType)
{
	VkShaderStageFlags flags = 0;
	if (has(shaderType, ShaderType::Vertex))
	{
		flags |= VK_SHADER_STAGE_VERTEX_BIT;
	}
	if (has(shaderType, ShaderType::Fragment))
	{
		flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	}
	if (has(shaderType, ShaderType::Compute))
	{
		flags |= VK_SHADER_STAGE_COMPUTE_BIT;
	}
	if (has(shaderType, ShaderType::Geometry))
	{
		flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
	}
	return flags;
}

bool valid(ShaderBindingType binding, BufferType buffer)
{
	switch (binding)
	{
	case aka::ShaderBindingType::UniformBuffer:
		return buffer == BufferType::Uniform;
	case aka::ShaderBindingType::StorageBuffer:
		return buffer == BufferType::ShaderStorage;
	default:
		return false;
	}
}

void VulkanProgram::updateDescriptorSet(VkDevice device, const Material* material)
{
	const VulkanMaterial* vk_material = reinterpret_cast<const VulkanMaterial*>(material);
	VulkanProgram* vk_program = reinterpret_cast<VulkanProgram*>(material->program);
	if (vk_material->vk_descriptorSet == VK_NULL_HANDLE)
		return;
	// TODO create a bindDescriptor / bindProgram
	std::vector<VkWriteDescriptorSet> descriptorWrites(vk_program->bindings.count, VkWriteDescriptorSet{});
	std::vector<VkDescriptorImageInfo> imageDescriptors(vk_program->bindings.count, VkDescriptorImageInfo{});
	std::vector<VkDescriptorBufferInfo> bufferDescriptors(vk_program->bindings.count, VkDescriptorBufferInfo{});
	uint32_t imageIndex = 0;
	uint32_t bufferIndex = 0;
	uint32_t samplerIndex = 0;

	for (uint32_t iBinding = 0; iBinding < vk_program->bindings.count; iBinding++)
	{
		const ShaderBindingLayout& binding = vk_program->bindings.bindings[iBinding];
		descriptorWrites[iBinding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[iBinding].dstSet = vk_material->vk_descriptorSet;
		descriptorWrites[iBinding].dstBinding = static_cast<uint32_t>(iBinding);
		descriptorWrites[iBinding].dstArrayElement = 0;
		descriptorWrites[iBinding].descriptorType = tovk(binding.type);
		descriptorWrites[iBinding].descriptorCount = binding.count;

		switch (binding.type)
		{
		case ShaderBindingType::SampledImage: {
			Texture* texture = material->images[iBinding];
			Sampler* sampler = material->samplers[iBinding];
			AKA_ASSERT(texture->type == TextureType::Texture2D || texture->type == TextureType::TextureCubeMap, "Invalid texture binding, skipping.");
			VkDescriptorImageInfo& vk_image = imageDescriptors[imageIndex++];
			vk_image.imageView = reinterpret_cast<VulkanTexture*>(texture)->vk_view;
			vk_image.sampler = reinterpret_cast<VulkanSampler*>(sampler)->vk_sampler;
			vk_image.imageLayout = reinterpret_cast<VulkanTexture*>(texture)->vk_layout;
			descriptorWrites[iBinding].pImageInfo = &vk_image;
			break;
		}
		case ShaderBindingType::StorageImage: {
			Texture* texture = material->images[iBinding];
			AKA_ASSERT(texture->type == TextureType::Texture2D, "Invalid texture binding, skipping.");
			VkDescriptorImageInfo& vk_image = imageDescriptors[imageIndex++];
			vk_image.imageView = reinterpret_cast<VulkanTexture*>(texture)->vk_view;
			vk_image.sampler = VK_NULL_HANDLE;
			vk_image.imageLayout = reinterpret_cast<VulkanTexture*>(texture)->vk_layout;
			descriptorWrites[iBinding].pImageInfo = &vk_image;
			break;
		}
		case ShaderBindingType::StorageBuffer:
		case ShaderBindingType::UniformBuffer: {
			Buffer* buffer = material->buffers[iBinding];
			AKA_ASSERT(valid(binding.type, buffer->type), "Invalid buffer binding, skipping.");
			VkDescriptorBufferInfo& vk_buffer = bufferDescriptors[bufferIndex++];
			vk_buffer.buffer = reinterpret_cast<VulkanBuffer*>(buffer)->vk_buffer;
			vk_buffer.offset = 0; // TODO use buffer view
			vk_buffer.range = buffer->size;
			descriptorWrites[iBinding].pBufferInfo = &vk_buffer;
			break;
		}
		default:
			AKA_ASSERT(false, "Invalid binding, skipping.");
			break;
		}
	}
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

VkDescriptorSetLayout VulkanProgram::createVkDescriptorSetLayout(VkDevice device, const ShaderBindingState& bindings, VkDescriptorPool* pool)
{
	if (bindings.count == 0)
		return VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayoutBinding> vk_bindings(bindings.count, VkDescriptorSetLayoutBinding{});
	std::vector<VkDescriptorPoolSize> vk_poolSizes(bindings.count, VkDescriptorPoolSize{});

	for (uint32_t i = 0; i < bindings.count; i++)
	{
		const ShaderBindingLayout& binding = bindings.bindings[i];
		vk_bindings[i].binding = i;
		vk_bindings[i].descriptorCount = binding.count;
		vk_bindings[i].descriptorType = tovk(binding.type);
		vk_bindings[i].pImmutableSamplers = nullptr;
		vk_bindings[i].stageFlags = tovk(binding.shaderType);

		// Pool
		vk_poolSizes[i].type = vk_bindings[i].descriptorType;
		vk_poolSizes[i].descriptorCount = vk_bindings[i].descriptorCount;
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(vk_bindings.size());
	layoutInfo.pBindings = vk_bindings.data();

	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(vk_poolSizes.size());
	poolInfo.pPoolSizes = vk_poolSizes.data();
	poolInfo.maxSets = 3 * 512; // TODO swapchain image count
	// TODO max instance count ? non optimal for memory

	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &poolInfo, nullptr, pool));

	return descriptorSetLayout;
}

VkDescriptorSet VulkanProgram::createVkDescriptorSet(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout* layouts, uint32_t layoutCount)
{
	if (layoutCount == 0)
		return VK_NULL_HANDLE;
	VkDescriptorSetAllocateInfo allocInfo {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool;
	allocInfo.pSetLayouts = layouts;
	allocInfo.descriptorSetCount = layoutCount;

	VkDescriptorSet set;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &set));
	return set;
}

Shader* VulkanGraphicDevice::compile(ShaderType type, const uint8_t* bytes, size_t size)
{	
	if (size == 0 || bytes == nullptr)
		return nullptr;
	VkShaderModuleCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(bytes);

	VkShaderModule vk_module;
	VK_CHECK_RESULT(vkCreateShaderModule(m_context.device, &createInfo, nullptr, &vk_module));

	return makeShader(type, vk_module);
}
void VulkanGraphicDevice::destroy(Shader* shader)
{
	VulkanShader* vk_shader = reinterpret_cast<VulkanShader*>(shader);
	vkDestroyShaderModule(m_context.device, vk_shader->vk_module, nullptr);
	m_shaderPool.release(vk_shader);
}

// Programs
Program* VulkanGraphicDevice::createProgram(Shader* vertex, Shader* fragment, Shader* geometry, const ShaderBindingState& bindings)
{
	// TODO check shaders
	if (bindings.count > ShaderBindingState::MaxBindingCount)
		return nullptr;
	VulkanProgram* vk_program = m_programPool.acquire();
	vk_program->vertex = vertex;
	vk_program->fragment = fragment;
	vk_program->compute = nullptr;
	vk_program->geometry = geometry;
	vk_program->bindings = bindings;

	vk_program->vk_descriptorPool = VK_NULL_HANDLE;
	vk_program->vk_descriptorSetLayout = VulkanProgram::createVkDescriptorSetLayout(m_context.device, vk_program->bindings, &vk_program->vk_descriptorPool);
	//vk_program->vk_descriptorSet = VulkanProgram::createVkDescriptorSet(m_context.device, vk_program->vk_descriptorPool, &vk_program->vk_descriptorSetLayout, (vk_program->vk_descriptorSetLayout == VK_NULL_HANDLE ? 0 : 1));
	vk_program->vk_pipelineLayout = m_context.getDescriptorLayout(bindings).pipelineLayout;

	vk_program->native = nullptr;// vk_program->vk_descriptorSet;

	return vk_program;
}

void VulkanGraphicDevice::destroy(Program* program)
{
	VulkanProgram* vk_program = reinterpret_cast<VulkanProgram*>(program);
	// only call free if VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT set
	//if (vk_program->vk_descriptorSet)
	//	vkFreeDescriptorSets(m_context.device, vk_program->vk_descriptorPool, 1, &vk_program->vk_descriptorSet);
	if (vk_program->vk_descriptorPool)
		vkDestroyDescriptorPool(m_context.device, vk_program->vk_descriptorPool, nullptr);
	if (vk_program->vk_descriptorSetLayout)
		vkDestroyDescriptorSetLayout(m_context.device, vk_program->vk_descriptorSetLayout, nullptr);
	m_programPool.release(vk_program);
}

Material* VulkanGraphicDevice::createMaterial(Program* program)
{
	if (program == nullptr)
		return nullptr;

	VulkanProgram* vk_program = reinterpret_cast<VulkanProgram*>(program);

	VulkanMaterial* material = m_materialPool.acquire();

	material->program = program;

	memset(material->images, 0x00, sizeof(material->images));
	memset(material->samplers, 0x00, sizeof(material->samplers));
	memset(material->buffers, 0x00, sizeof(material->buffers));

	material->vk_descriptorSet = VulkanProgram::createVkDescriptorSet(
		m_context.device, 
		vk_program->vk_descriptorPool,
		&vk_program->vk_descriptorSetLayout, 
		(vk_program->vk_descriptorSetLayout == VK_NULL_HANDLE ? 0 : 1)
	);

	return material;
}

void VulkanGraphicDevice::destroy(Material* material)
{
	VulkanMaterial* vk_material = reinterpret_cast<VulkanMaterial*>(material);
	// only call free if VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT set
	//if (vk_material->vk_descriptorSet)
	//	vkFreeDescriptorSets(m_context.device, vk_material->vk_descriptorPool, 1, &vk_material->vk_descriptorSet);
	m_materialPool.release(vk_material);
}

VulkanShader* VulkanGraphicDevice::makeShader(ShaderType type, VkShaderModule module)
{
	VulkanShader* shader = m_shaderPool.acquire();
	shader->type = type;

	shader->vk_module = module;

	// Set native handle for others API
	shader->native = module;
	return shader;
}

};