#include "VulkanBuffer.h"
#include "VulkanGraphicDevice.h"

namespace aka {

VkBuffer VulkanBuffer::createVkBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer vk_buffer = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &vk_buffer));
	return vk_buffer;
}

VkDeviceMemory VulkanBuffer::createVkDeviceMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer vk_buffer, VkMemoryPropertyFlags properties)
{
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, vk_buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = VulkanContext::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

	VkDeviceMemory vk_memory = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &vk_memory));

	VK_CHECK_RESULT(vkBindBufferMemory(device, vk_buffer, vk_memory, 0));
	return vk_memory;
}

Buffer* VulkanGraphicDevice::createBuffer(BufferType type, uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;// VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; // TODO depend on access
	VkBufferUsageFlags usages = VulkanContext::tovk(type); // VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT
	
	VkBuffer vk_buffer = VulkanBuffer::createVkBuffer(m_context.device, size, usages);
	VkDeviceMemory vk_memory = VulkanBuffer::createVkDeviceMemory(m_context.device, m_context.physicalDevice, vk_buffer, properties);
		
	VulkanBuffer* buffer = makeBuffer(
		type,
		size,
		usage,
		access,
		vk_buffer,
		vk_memory
	);

	if (data != nullptr)
	{
		upload(buffer, data, 0, size);
	}
	return buffer;
}

void VulkanGraphicDevice::upload(const Buffer* buffer, const void* data, uint32_t offset, uint32_t size)
{
	// TODO staging buffer for device local buffers
	const VulkanBuffer* vk_buffer = reinterpret_cast<const VulkanBuffer*>(buffer);
	void* mapped = nullptr;
	VK_CHECK_RESULT(vkMapMemory(m_context.device, vk_buffer->vk_memory, offset, size, 0, &mapped));
	if (mapped == nullptr)
	{
		Logger::error("Failed to map memory.");
		return;
	}
	memcpy(mapped, data, static_cast<size_t>(size));
	vkUnmapMemory(m_context.device, vk_buffer->vk_memory);
}

void VulkanGraphicDevice::download(const Buffer* buffer, void* data, uint32_t offset, uint32_t size)
{
	const VulkanBuffer* vk_buffer = reinterpret_cast<const VulkanBuffer*>(buffer);
	void* mapped = nullptr;
	VK_CHECK_RESULT(vkMapMemory(m_context.device, vk_buffer->vk_memory, offset, size, 0, &mapped));
	if (mapped == nullptr)
	{
		Logger::error("Failed to map memory.");
		return;
	}
	memcpy(data, mapped, static_cast<size_t>(size));
	vkUnmapMemory(m_context.device, vk_buffer->vk_memory);
}

void* VulkanGraphicDevice::map(Buffer* buffer, BufferMap map)
{
	VulkanBuffer* vk_buffer = reinterpret_cast<VulkanBuffer*>(buffer);
	void* data = nullptr;
	VK_CHECK_RESULT(vkMapMemory(m_context.device, vk_buffer->vk_memory, 0, vk_buffer->size, 0, &data));
	return data;
}

void VulkanGraphicDevice::unmap(Buffer* buffer)
{
	// TODO ensure its a vk texture
	VulkanBuffer* vk_buffer = reinterpret_cast<VulkanBuffer*>(buffer);
	vkUnmapMemory(m_context.device, vk_buffer->vk_memory);
}

void VulkanGraphicDevice::destroy(Buffer* buffer)
{
	if (buffer == nullptr)
		return;
	VulkanBuffer* vk_buffer = reinterpret_cast<VulkanBuffer*>(buffer);
	vkFreeMemory(m_context.device, vk_buffer->vk_memory, nullptr);
	vkDestroyBuffer(m_context.device, vk_buffer->vk_buffer, nullptr);
	m_bufferPool.release(vk_buffer);
}

VulkanBuffer* VulkanGraphicDevice::makeBuffer(BufferType type, uint32_t size, BufferUsage usage, BufferCPUAccess access, VkBuffer vk_buffer, VkDeviceMemory vk_memory)
{
	VulkanBuffer* buffer = m_bufferPool.acquire();
	buffer->size = size;
	buffer->usage = usage;
	buffer->type = type;
	buffer->access = access;
	buffer->type = type;

	buffer->vk_buffer = vk_buffer;
	buffer->vk_memory = vk_memory;

	// Set native handle for others API
	buffer->native = buffer;
	return buffer;
}

};