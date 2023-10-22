#include "VulkanProgram.h"

#include "VulkanGraphicDevice.h"

namespace aka {
namespace gfx {

VkDescriptorType VulkanContext::tovk(ShaderBindingType type)
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

VkShaderStageFlags VulkanContext::tovk(ShaderMask shaderType)
{
	VkShaderStageFlags flags = 0;
	if (has(shaderType, ShaderMask::Vertex))
	{
		flags |= VK_SHADER_STAGE_VERTEX_BIT;
	}
	if (has(shaderType, ShaderMask::Fragment))
	{
		flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	}
	if (has(shaderType, ShaderMask::Compute))
	{
		flags |= VK_SHADER_STAGE_COMPUTE_BIT;
	}
	if (has(shaderType, ShaderMask::Geometry))
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
		return buffer == BufferType::Storage;
	default:
		return false;
	}
}

VulkanProgram::VulkanProgram(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderBindingState* sets, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount) :
	Program(name, vertex, fragment, sets, bindingCounts, constants, constantCount)
{
}

VulkanProgram::VulkanProgram(const char* name, ShaderHandle vertex, ShaderHandle fragment, ShaderHandle geometry, const ShaderBindingState* sets, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount) :
	Program(name, vertex, fragment, geometry, sets, bindingCounts, constants, constantCount)
{
}

VulkanProgram::VulkanProgram(const char* name, ShaderHandle compute, const ShaderBindingState* sets, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount) :
	Program(name, compute, sets, bindingCounts, constants, constantCount)
{
}

void VulkanDescriptorSet::updateDescriptorSet(VulkanGraphicDevice* device, const DescriptorSet* set, const DescriptorUpdate* updates, size_t size)
{
	const VulkanDescriptorSet* vk_set = reinterpret_cast<const VulkanDescriptorSet*>(set);
	if (vk_set->vk_descriptorSet == VK_NULL_HANDLE)
		return;
	std::vector<VkWriteDescriptorSet> descriptorWrites(size, VkWriteDescriptorSet{});
	std::vector<VkDescriptorImageInfo> imageDescriptors;
	std::vector<VkDescriptorBufferInfo> bufferDescriptors;
	imageDescriptors.reserve(size);
	bufferDescriptors.reserve(size);

	for (size_t iUpdate = 0; iUpdate < size; iUpdate++)
	{
		const DescriptorUpdate& update = updates[iUpdate];
		const ShaderBindingLayout& binding = vk_set->bindings.bindings[update.binding];
		descriptorWrites[iUpdate].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[iUpdate].dstSet = vk_set->vk_descriptorSet;
		descriptorWrites[iUpdate].dstBinding = update.binding;
		descriptorWrites[iUpdate].dstArrayElement = update.index;
		descriptorWrites[iUpdate].descriptorType = VulkanContext::tovk(binding.type);
		descriptorWrites[iUpdate].descriptorCount = 1; // Here we update only one desc set, so do not use binding.count;

		switch (binding.type)
		{
		case ShaderBindingType::SampledImage: {
			VulkanTexture* vk_texture = device->getVk<VulkanTexture>(update.texture.texture);
			VulkanSampler* vk_sampler = device->getVk<VulkanSampler>(update.texture.sampler);
			AKA_ASSERT(
				vk_texture->type == TextureType::Texture2D ||
				vk_texture->type == TextureType::TextureCubeMap ||
				vk_texture->type == TextureType::Texture2DArray,
				"Invalid texture binding, skipping."
			);
			VkDescriptorImageInfo& vk_image = imageDescriptors.emplace_back();
			vk_image.imageView = vk_texture->getImageView(device, update.texture.layer, update.texture.mipLevel);
			vk_image.sampler = vk_sampler->vk_sampler;
			vk_image.imageLayout = VulkanContext::tovk(ResourceAccessType::Resource, vk_texture->format);
			descriptorWrites[iUpdate].pImageInfo = &vk_image;
			break;
		}
		case ShaderBindingType::StorageImage: {
			VulkanTexture* vk_texture = device->getVk<VulkanTexture>(update.texture.texture);
			AKA_ASSERT(vk_texture->type == TextureType::Texture2D, "Invalid texture binding, skipping.");
			VkDescriptorImageInfo& vk_image = imageDescriptors.emplace_back();
			vk_image.imageView = vk_texture->getImageView(device, update.texture.layer, update.texture.mipLevel);
			vk_image.sampler = VK_NULL_HANDLE;
			vk_image.imageLayout = VulkanContext::tovk(ResourceAccessType::Storage, vk_texture->format);
			descriptorWrites[iUpdate].pImageInfo = &vk_image;
			break;
		}
		case ShaderBindingType::StorageBuffer:
		case ShaderBindingType::UniformBuffer: {
			VulkanBuffer* buffer = device->getVk<VulkanBuffer>(update.buffer.handle);
			VkDescriptorBufferInfo& vk_buffer = bufferDescriptors.emplace_back();
			if (buffer == nullptr) // No buffer
			{
				vk_buffer.buffer = VK_NULL_HANDLE;
				vk_buffer.offset = 0;
				vk_buffer.range = 0;
			}
			else
			{
				VkPhysicalDeviceProperties properties{};
				vkGetPhysicalDeviceProperties(device->getVkPhysicalDevice(), &properties);
				AKA_ASSERT(
					(update.buffer.offset % properties.limits.minStorageBufferOffsetAlignment == 0) && (buffer->type == gfx::BufferType::Storage) ||
					(update.buffer.offset % properties.limits.minUniformBufferOffsetAlignment == 0) && (buffer->type == gfx::BufferType::Uniform),
					"Invalid offset"
				);
				AKA_ASSERT(valid(binding.type, buffer->type), "Invalid buffer binding, skipping.");
				vk_buffer.buffer = reinterpret_cast<const VulkanBuffer*>(buffer)->vk_buffer;
				vk_buffer.offset = update.buffer.offset;
				vk_buffer.range = (update.buffer.range == ~0U) ? VK_WHOLE_SIZE : update.buffer.range;
			}
			descriptorWrites[iUpdate].pBufferInfo = &vk_buffer;
			break;
		}
		default:
			AKA_ASSERT(false, "Invalid binding, skipping.");
			break;
		}
	}
	// Might need to pack same binding in single write.
	vkUpdateDescriptorSets(device->getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

VkDescriptorSetLayout VulkanDescriptorSet::createVkDescriptorSetLayout(VkDevice device, const ShaderBindingState& bindings)
{
	AKA_ASSERT(bindings.count > 0, "Invalid inputs");
	Vector<VkDescriptorSetLayoutBinding> vk_bindings(bindings.count, VkDescriptorSetLayoutBinding{});
	Vector<VkDescriptorBindingFlags> vk_flags(bindings.count, VkDescriptorBindingFlags{});
	bool allowUpdateAfterBind = false;
	for (uint32_t i = 0; i < bindings.count; i++)
	{
		const ShaderBindingLayout& binding = bindings.bindings[i];
		vk_bindings[i].binding = i;
		vk_bindings[i].descriptorCount = binding.count;
		vk_bindings[i].descriptorType = VulkanContext::tovk(binding.type);
		vk_bindings[i].pImmutableSamplers = nullptr;
		vk_bindings[i].stageFlags = VulkanContext::tovk(binding.stages);
		if (asBool(binding.flags & ShaderBindingFlag::Bindless))
			vk_flags[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT; // Allow null values within array
		if (asBool(binding.flags & ShaderBindingFlag::UpdateAfterBind))
			vk_flags[i] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT; // Allow update after the table is bound.
		allowUpdateAfterBind |= vk_flags[i] & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}

	VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
	bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	bindingFlags.bindingCount = (uint32_t)vk_flags.size();
	bindingFlags.pBindingFlags = vk_flags.data();

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(vk_bindings.size());
	layoutInfo.pBindings = vk_bindings.data();
	layoutInfo.pNext = &bindingFlags;
	if (allowUpdateAfterBind)
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));
	return descriptorSetLayout;
}
VkDescriptorPool VulkanDescriptorPool::createVkDescriptorPool(VkDevice device, const ShaderBindingState& bindings, uint32_t size)
{
	AKA_ASSERT(bindings.count > 0, "Invalid inputs");
	std::vector<VkDescriptorPoolSize> vk_poolSizes(bindings.count, VkDescriptorPoolSize{});

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	for (uint32_t i = 0; i < bindings.count; i++)
	{
		const ShaderBindingLayout& binding = bindings.bindings[i];
		vk_poolSizes[i].type = VulkanContext::tovk(binding.type);
		vk_poolSizes[i].descriptorCount = 1;// binding.count;
		if (asBool(binding.flags & ShaderBindingFlag::UpdateAfterBind))
			poolInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	}
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(vk_poolSizes.size());
	poolInfo.pPoolSizes = vk_poolSizes.data();
	poolInfo.maxSets = size;

	VkDescriptorPool pool = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool));
	return pool;
}
// https://jorenjoestar.github.io/post/vulkan_bindless_texture/
// https://dev.to/gasim/implementing-bindless-design-in-vulkan-34no
VkDescriptorSet VulkanDescriptorSet::createVkDescriptorSet(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout* layouts, uint32_t layoutCount)
{
	if (layoutCount == 0)
		return VK_NULL_HANDLE;
	VkDescriptorSetAllocateInfo allocInfo {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = layoutCount;
	allocInfo.pSetLayouts = layouts;

	VkDescriptorSet set;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &set));
	return set;
}

ShaderHandle VulkanGraphicDevice::createShader(const char* name, ShaderType type, const void* bytes, size_t size)
{	
	if (size == 0 || bytes == nullptr)
		return ShaderHandle::null;
	VkShaderModuleCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(bytes);

	VulkanShader* vk_shader = m_shaderPool.acquire(name, type);
	VK_CHECK_RESULT(vkCreateShaderModule(m_context.device, &createInfo, nullptr, &vk_shader->vk_module));

	setDebugName(m_context.device, vk_shader->vk_module, name);
	return ShaderHandle{ vk_shader };
}
void VulkanGraphicDevice::destroy(ShaderHandle shader)
{
	if (get(shader) == nullptr) return;

	VulkanShader* vk_shader = getVk<VulkanShader>(shader);
	vkDestroyShaderModule(m_context.device, vk_shader->vk_module, nullptr);

	m_shaderPool.release(vk_shader);
}

const Shader* VulkanGraphicDevice::get(ShaderHandle handle)
{
	return static_cast<const Shader*>(handle.__data);
}

VulkanShader::VulkanShader(const char* name, ShaderType type) :
	Shader(name, type),
	vk_module(VK_NULL_HANDLE)
{
}

// -----------------------------------------------------------------

ProgramHandle VulkanGraphicDevice::createGraphicProgram(const char* name, ShaderHandle vertex, ShaderHandle fragment, ShaderHandle geometry, const ShaderBindingState* sets, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount)
{
	// TODO check shaders
	if (bindingCounts > ShaderMaxSetCount)
		return ProgramHandle::null;
	if (sets[0].count > ShaderMaxBindingCount)
		return ProgramHandle::null;
	VulkanProgram* vk_program = m_programPool.acquire(name, vertex, fragment, geometry, sets, bindingCounts, constants, constantCount);

	// Create vk data
	for (uint32_t i = 0; i < bindingCounts; i++)
	{
		//vk_program->vk_descriptorSetLayout[i] = m_context.getDescriptorSetLayout(vk_program->sets[i]);
	}

	vk_program->native = 0;// reinterpret_cast<std::uintptr_t>(vk_program->vk_descriptorSet);

	return ProgramHandle{ vk_program };
}
ProgramHandle VulkanGraphicDevice::createComputeProgram(const char* name, ShaderHandle compute, const ShaderBindingState* sets, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount)
{
	// TODO check shaders
	if (bindingCounts > ShaderMaxSetCount)
		return ProgramHandle::null;
	if (sets[0].count > ShaderMaxBindingCount)
		return ProgramHandle::null;
	VulkanProgram* vk_program = m_programPool.acquire(name, compute, sets, bindingCounts, constants, constantCount);

	// Create vk data
	for (uint32_t i = 0; i < bindingCounts; i++)
	{
		//vk_program->vk_descriptorSetLayout[i] = m_context.getDescriptorSetLayout(vk_program->sets[i]);
	}

	vk_program->native = 0;// reinterpret_cast<std::uintptr_t>(vk_program->vk_descriptorSet);

	return ProgramHandle{ vk_program };
}

void VulkanGraphicDevice::destroy(ProgramHandle program)
{
	if (get(program) == nullptr) return;

	VulkanProgram* vk_program = getVk<VulkanProgram>(program);
	for (uint32_t i = 0; i < vk_program->setCount; i++)
	{
		// This is stored in cache so should only unreference it
		//if (vk_program->vk_descriptorSetLayout[i])
		//	vkDestroyDescriptorSetLayout(m_context.device, vk_program->vk_descriptorSetLayout[i], nullptr);
	}
	m_programPool.release(vk_program);
}

const Program* VulkanGraphicDevice::get(ProgramHandle handle)
{
	return static_cast<const Program*>(handle.__data);
}

// -----------------------------------------------------------------

DescriptorPoolHandle VulkanGraphicDevice::createDescriptorPool(const char* name, const ShaderBindingState& bindings, uint32_t size)
{
	VulkanDescriptorPool* vk_pool = m_descriptorPoolPool.acquire(name, bindings, size);
	vk_pool->create(m_context);

	return DescriptorPoolHandle{ vk_pool };
}

const DescriptorPool* VulkanGraphicDevice::get(DescriptorPoolHandle handle)
{
	return static_cast<const DescriptorPool*>(handle.__data);
}

void VulkanGraphicDevice::destroy(DescriptorPoolHandle pool)
{
	VulkanDescriptorPool* vk_pool = getVk<VulkanDescriptorPool>(pool);
	vk_pool->destroy(m_context);
	m_descriptorPoolPool.release(vk_pool);
}

VulkanDescriptorPool::VulkanDescriptorPool(const char* name, const ShaderBindingState& bindings, uint32_t size) :
	DescriptorPool(name, bindings, size),
	vk_pool(VK_NULL_HANDLE)
{
}

void VulkanDescriptorPool::create(VulkanContext& context)
{
	vk_pool = VulkanDescriptorPool::createVkDescriptorPool(context.device, bindings, size);
}

void VulkanDescriptorPool::destroy(VulkanContext& context)
{
	vkDestroyDescriptorPool(context.device, vk_pool, nullptr);
}

// -----------------------------------------------------------------

DescriptorSetHandle VulkanGraphicDevice::allocateDescriptorSet(const char* name, const ShaderBindingState& bindings, DescriptorPoolHandle pool)
{
	VulkanDescriptorSet* vk_descriptor = m_descriptorSetPool.acquire(name, bindings, pool);

	vk_descriptor->vk_pool = getVk<VulkanDescriptorPool>(pool)->vk_pool;
	vk_descriptor->create(m_context);

	return DescriptorSetHandle{ vk_descriptor };
}
void VulkanGraphicDevice::update(DescriptorSetHandle descriptorSet, const DescriptorUpdate* update, size_t size)
{
	auto bindings = getVk<VulkanDescriptorSet>(descriptorSet)->bindings;
	VulkanDescriptorSet::updateDescriptorSet(this, get(descriptorSet), update, size);
}

void VulkanGraphicDevice::free(DescriptorSetHandle descriptorSet)
{
	VulkanDescriptorSet* vk_descriptor = getVk<VulkanDescriptorSet>(descriptorSet);
	vk_descriptor->destroy(m_context);
	m_descriptorSetPool.release(vk_descriptor);
}

const DescriptorSet* VulkanGraphicDevice::get(DescriptorSetHandle handle)
{
	return static_cast<const DescriptorSet*>(handle.__data);
}

VulkanDescriptorSet::VulkanDescriptorSet(const char* name, const ShaderBindingState& bindings, DescriptorPoolHandle pool) :
	DescriptorSet(name, bindings, pool),
	vk_descriptorSet(VK_NULL_HANDLE)
{
}

void VulkanDescriptorSet::create(VulkanContext& context)
{
	VkDescriptorSetLayout layout = context.getDescriptorSetLayout(bindings);
	vk_descriptorSet = VulkanDescriptorSet::createVkDescriptorSet(
		context.device,
		vk_pool,
		&layout,
		(layout == VK_NULL_HANDLE ? 0 : 1)
	);
	setDebugName(context.device, vk_descriptorSet, "VkDescriptorSet_", name);
}

void VulkanDescriptorSet::destroy(VulkanContext& context)
{
	// only call free if VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT set
	VK_CHECK_RESULT(vkFreeDescriptorSets(context.device, vk_pool, 1, &vk_descriptorSet));
	vk_descriptorSet = VK_NULL_HANDLE;
	vk_pool = VK_NULL_HANDLE; // Do not own
}

};
};