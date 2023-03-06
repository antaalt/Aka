#include "VulkanBuffer.h"
#include "VulkanGraphicDevice.h"

namespace aka {
namespace gfx {

VulkanBuffer::VulkanBuffer(const char* name, BufferType type, uint32_t size, BufferUsage usage, BufferCPUAccess access) :
	Buffer(name, type, size, usage, access),
	vk_buffer(VK_NULL_HANDLE),
	vk_memory(VK_NULL_HANDLE)
{
}
void VulkanBuffer::create(VulkanContext& context)
{
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;// VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; // TODO depend on access
	VkBufferUsageFlags usages = VulkanContext::tovk(type); // VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT

	vk_buffer = VulkanBuffer::createVkBuffer(context.device, size, usages);
	vk_memory = VulkanBuffer::createVkDeviceMemory(context.device, context.physicalDevice, vk_buffer, properties);

	setDebugName(context.device, vk_buffer, "VkBuffer_", name);
	setDebugName(context.device, vk_buffer, "VkDeviceMemory_", name);
}
void VulkanBuffer::destroy(VulkanContext& context)
{
	vkFreeMemory(context.device, vk_memory, nullptr);
	vkDestroyBuffer(context.device, vk_buffer, nullptr);
	vk_buffer = VK_NULL_HANDLE;
	vk_memory = VK_NULL_HANDLE;
}
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

BufferHandle VulkanGraphicDevice::createBuffer(const char* name, BufferType type, uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{		
	VulkanBuffer* buffer = m_bufferPool.acquire(name, type, size, usage, access);

	buffer->create(m_context);

	if (data != nullptr)
	{
		upload(BufferHandle{ buffer }, data, 0, size);
	}
	return BufferHandle{ buffer };
}

void VulkanGraphicDevice::upload(BufferHandle buffer, const void* data, uint32_t offset, uint32_t size)
{
	// TODO staging buffer for device local buffers
	VulkanBuffer* vk_buffer = getVk<VulkanBuffer>(buffer);
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

void VulkanGraphicDevice::download(BufferHandle buffer, void* data, uint32_t offset, uint32_t size)
{
	VulkanBuffer* vk_buffer = getVk<VulkanBuffer>(buffer);
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

void* VulkanGraphicDevice::map(BufferHandle buffer, BufferMap map)
{
	VulkanBuffer* vk_buffer = getVk<VulkanBuffer>(buffer);
	void* data = nullptr;
	VK_CHECK_RESULT(vkMapMemory(m_context.device, vk_buffer->vk_memory, 0, vk_buffer->size, 0, &data));
	return data;
}

void VulkanGraphicDevice::unmap(BufferHandle buffer)
{
	// TODO ensure its a vk texture
	VulkanBuffer* vk_buffer = getVk<VulkanBuffer>(buffer);
	vkUnmapMemory(m_context.device, vk_buffer->vk_memory);
}

void VulkanGraphicDevice::destroy(BufferHandle buffer)
{
	if (get(buffer) == nullptr) return;

	VulkanBuffer* vk_buffer = getVk<VulkanBuffer>(buffer);
	vk_buffer->destroy(m_context);
	m_bufferPool.release(vk_buffer);
}

const Buffer* VulkanGraphicDevice::get(BufferHandle buffer)
{
	return buffer.__data; // TODO map somehow
}

};
};