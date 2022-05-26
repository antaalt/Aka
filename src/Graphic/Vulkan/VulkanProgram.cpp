#include "VulkanProgram.h"

#include "VulkanGraphicDevice.h"

namespace aka {
namespace gfx {

VkDescriptorType tovk(ShaderBindingType type)
{
	switch (type)
	{
	default:
	case ShaderBindingType::SampledImage:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	case ShaderBindingType::StorageImage:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case ShaderBindingType::UniformBuffer:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case ShaderBindingType::StorageBuffer:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case ShaderBindingType::AccelerationStructure:
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
	case ShaderBindingType::UniformBuffer:
		return buffer == BufferType::Uniform;
	case ShaderBindingType::StorageBuffer:
		return buffer == BufferType::ShaderStorage;
	default:
		return false;
	}
}

void VulkanProgram::updateDescriptorSet(VkDevice device, const DescriptorSet* set, const DescriptorSetData& data)
{
	const VulkanDescriptorSet* vk_set = reinterpret_cast<const VulkanDescriptorSet*>(set);
	if (vk_set->vk_descriptorSet == VK_NULL_HANDLE)
		return;
	// TODO create a bindDescriptor / bindProgram
	std::vector<VkWriteDescriptorSet> descriptorWrites(vk_set->bindings.count, VkWriteDescriptorSet{});
	std::vector<VkDescriptorImageInfo> imageDescriptors(vk_set->bindings.count, VkDescriptorImageInfo{});
	std::vector<VkDescriptorBufferInfo> bufferDescriptors(vk_set->bindings.count, VkDescriptorBufferInfo{});
	uint32_t imageIndex = 0;
	uint32_t bufferIndex = 0;
	uint32_t samplerIndex = 0;

	for (uint32_t iBinding = 0; iBinding < vk_set->bindings.count; iBinding++)
	{
		const ShaderBindingLayout& binding = vk_set->bindings.bindings[iBinding];
		descriptorWrites[iBinding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[iBinding].dstSet = vk_set->vk_descriptorSet;
		descriptorWrites[iBinding].dstBinding = static_cast<uint32_t>(iBinding);
		descriptorWrites[iBinding].dstArrayElement = 0;
		descriptorWrites[iBinding].descriptorType = tovk(binding.type);
		descriptorWrites[iBinding].descriptorCount = binding.count;

		switch (binding.type)
		{
		case ShaderBindingType::SampledImage: {
			VulkanTexture* texture = get<VulkanTexture>(data.images[iBinding]);
			VulkanSampler* sampler = get<VulkanSampler>(data.samplers[iBinding]);
			AKA_ASSERT(
				texture->type == TextureType::Texture2D || 
				texture->type == TextureType::TextureCubeMap || 
				texture->type == TextureType::Texture2DArray,
				"Invalid texture binding, skipping."
			);
			VkDescriptorImageInfo& vk_image = imageDescriptors[imageIndex++];
			vk_image.imageView = reinterpret_cast<const VulkanTexture*>(texture)->vk_view;
			vk_image.sampler = reinterpret_cast<const VulkanSampler*>(sampler)->vk_sampler;
			vk_image.imageLayout = Texture::hasDepth(texture->format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;//reinterpret_cast<VulkanTexture*>(texture)->vk_layout;
			descriptorWrites[iBinding].pImageInfo = &vk_image;
			break;
		}
		case ShaderBindingType::StorageImage: {
			VulkanTexture* texture = get<VulkanTexture>(data.images[iBinding]);
			AKA_ASSERT(texture->type == TextureType::Texture2D, "Invalid texture binding, skipping.");
			VkDescriptorImageInfo& vk_image = imageDescriptors[imageIndex++];
			vk_image.imageView = reinterpret_cast<const VulkanTexture*>(texture)->vk_view;
			vk_image.sampler = VK_NULL_HANDLE;
			vk_image.imageLayout = Texture::hasDepth(texture->format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;//reinterpret_cast<VulkanTexture*>(texture)->vk_layout;
			descriptorWrites[iBinding].pImageInfo = &vk_image;
			break;
		}
		case ShaderBindingType::StorageBuffer:
		case ShaderBindingType::UniformBuffer: {
			VulkanBuffer* buffer = get<VulkanBuffer>(data.buffers[iBinding]);
			if (buffer == nullptr)
			{
				VkDescriptorBufferInfo& vk_buffer = bufferDescriptors[bufferIndex++];
				vk_buffer.buffer = VK_NULL_HANDLE;
				vk_buffer.offset = 0;
				vk_buffer.range = 0;
				descriptorWrites[iBinding].pBufferInfo = &vk_buffer;
			}
			else
			{
				AKA_ASSERT(valid(binding.type, buffer->type), "Invalid buffer binding, skipping.");
				VkDescriptorBufferInfo& vk_buffer = bufferDescriptors[bufferIndex++];
				vk_buffer.buffer = reinterpret_cast<const VulkanBuffer*>(buffer)->vk_buffer;
				vk_buffer.offset = 0; // TODO use buffer view
				vk_buffer.range = buffer->size;
				descriptorWrites[iBinding].pBufferInfo = &vk_buffer;
			}
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

ShaderHandle VulkanGraphicDevice::createShader(ShaderType type, const void* bytes, size_t size)
{	
	if (size == 0 || bytes == nullptr)
		return ShaderHandle::null;
	VkShaderModuleCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(bytes);

	VkShaderModule vk_module;
	VK_CHECK_RESULT(vkCreateShaderModule(m_context.device, &createInfo, nullptr, &vk_module));

	return ShaderHandle{ makeShader(type, vk_module) };
}
void VulkanGraphicDevice::destroy(ShaderHandle shader)
{
	if (shader.data == nullptr) return;

	VulkanShader* vk_shader = get<VulkanShader>(shader);
	vkDestroyShaderModule(m_context.device, vk_shader->vk_module, nullptr);

	m_shaderPool.release(vk_shader);
}

// Programs
ProgramHandle VulkanGraphicDevice::createProgram(ShaderHandle vertex, ShaderHandle fragment, ShaderHandle geometry, const ShaderBindingState* sets, uint32_t bindingCounts)
{
	// TODO check shaders
	if (bindingCounts == 0 || bindingCounts > ShaderMaxSetCount)
		return ProgramHandle::null;
	if (sets[0].count > ShaderMaxBindingCount)
		return ProgramHandle::null;
	VulkanProgram* vk_program = m_programPool.acquire();
	vk_program->vertex = vertex;
	vk_program->fragment = fragment;
	vk_program->compute = ShaderHandle::null;
	vk_program->geometry = geometry;
	memcpy(vk_program->sets, sets, bindingCounts * sizeof(ShaderBindingState));

	vk_program->setCount = bindingCounts;
	for (uint32_t i = 0; i < bindingCounts; i++)
	{
		auto data = m_context.getDescriptorLayout(vk_program->sets[i]);
		vk_program->vk_descriptorPool[i] = data.pool;
		vk_program->vk_descriptorSetLayout[i] = data.layout;
		//vk_program->vk_pipelineLayout = m_context.getPipelineLayout(vk_program->vk_descriptorSetLayout);
	}

	vk_program->native = 0;// reinterpret_cast<std::uintptr_t>(vk_program->vk_descriptorSet);

	return ProgramHandle{ vk_program };
}

void VulkanGraphicDevice::destroy(ProgramHandle program)
{
	if (program.data == nullptr) return;

	VulkanProgram* vk_program = get<VulkanProgram>(program);
	// only call free if VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT set
	//if (vk_program->vk_descriptorSet)
	//	vkFreeDescriptorSets(m_context.device, vk_program->vk_descriptorPool, 1, &vk_program->vk_descriptorSet);
	for (uint32_t i = 0; i < vk_program->setCount; i++)
	{
		// This is stored in cache so should only unreference it
		//if (vk_program->vk_descriptorPool[i])
		//	vkDestroyDescriptorPool(m_context.device, vk_program->vk_descriptorPool[i], nullptr);
		//if (vk_program->vk_descriptorSetLayout[i])
		//	vkDestroyDescriptorSetLayout(m_context.device, vk_program->vk_descriptorSetLayout[i], nullptr);
	}
	m_programPool.release(vk_program);
}

DescriptorSetHandle VulkanGraphicDevice::createDescriptorSet(const ShaderBindingState& bindings)
{
	VulkanDescriptorSet* material = m_descriptorPool.acquire();

	material->bindings = bindings;

	VulkanContext::ShaderInputData data = m_context.getDescriptorLayout(bindings);
	material->vk_descriptorSet = VulkanProgram::createVkDescriptorSet(
		m_context.device, 
		data.pool,
		&data.layout,
		(data.layout == VK_NULL_HANDLE ? 0 : 1)
	);

	return DescriptorSetHandle{ material };
}
void VulkanGraphicDevice::update(DescriptorSetHandle material, const DescriptorSetData& data)
{
	// TODO validate data & bindings
	VulkanProgram::updateDescriptorSet(m_context.device, material.data, data);
}

void VulkanGraphicDevice::destroy(DescriptorSetHandle material)
{
	const VulkanDescriptorSet* vk_material = reinterpret_cast<const VulkanDescriptorSet*>(material.data);
	// only call free if VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT set
	//if (vk_material->vk_descriptorSet)
	//	vkFreeDescriptorSets(m_context.device, vk_material->vk_descriptorPool, 1, &vk_material->vk_descriptorSet);
	m_descriptorPool.release(const_cast<VulkanDescriptorSet*>(vk_material));
}

VulkanShader* VulkanGraphicDevice::makeShader(ShaderType type, VkShaderModule module)
{
	VulkanShader* shader = m_shaderPool.acquire();
	shader->type = type;

	shader->vk_module = module;

	// Set native handle for others API
	shader->native = reinterpret_cast<std::uintptr_t>(module);
	return shader;
}

};
};