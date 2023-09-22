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
void VulkanBuffer::create(VulkanGraphicDevice* device)
{
	VkMemoryPropertyFlags properties = VulkanContext::tovk(usage);
	VkBufferUsageFlags usages = VulkanContext::tovk(type);

	if (usage != BufferUsage::Immutable)
	{
		usages |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		usages |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}

	vk_buffer = VulkanBuffer::createVkBuffer(device->getVkDevice(), size, usages);
	vk_memory = VulkanBuffer::createVkDeviceMemory(device->getVkDevice(), device->getVkPhysicalDevice(), vk_buffer, properties);

	setDebugName(device->getVkDevice(), vk_buffer, "VkBuffer_", name);
	setDebugName(device->getVkDevice(), vk_buffer, "VkDeviceMemory_", name);
}
void VulkanBuffer::destroy(VulkanGraphicDevice* device)
{
	vkFreeMemory(device->getVkDevice(), vk_memory, nullptr);
	vkDestroyBuffer(device->getVkDevice(), vk_buffer, nullptr);
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

void VulkanBuffer::insertMemoryBarrier(VkCommandBuffer command, VkBuffer buffer, size_t size, size_t offset, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
{
	VkBufferMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.buffer = buffer;
	barrier.offset = offset;
	barrier.size = size;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.srcAccessMask = srcAccess;
	barrier.dstAccessMask = dstAccess;

	vkCmdPipelineBarrier(
		command,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, &barrier,
		1, nullptr
	);
}

bool VulkanBuffer::isMappable(BufferUsage usage)
{
	switch (usage)
	{
	case BufferUsage::Dynamic:
	case BufferUsage::Staging:
		return true;
	default:
		return false;
	}
}

bool VulkanBuffer::isTransferable(BufferUsage usage)
{
	switch (usage)
	{
	case BufferUsage::Immutable:
		return false;
	default:
		return true;
	}
}

BufferHandle VulkanGraphicDevice::createBuffer(const char* name, BufferType type, uint32_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{		
	VulkanBuffer* buffer = m_bufferPool.acquire(name, type, size, usage, access);

	buffer->create(this);

	if (data != nullptr)
	{
		upload(BufferHandle{ buffer }, data, 0, size);
	}
	return BufferHandle{ buffer };
}

void VulkanGraphicDevice::upload(BufferHandle buffer, const void* data, uint32_t offset, uint32_t size)
{
	VulkanBuffer* vk_buffer = getVk<VulkanBuffer>(buffer);
	if (VulkanBuffer::isMappable(vk_buffer->usage))
	{
		void* mapped = nullptr;
		VK_CHECK_RESULT(vkMapMemory(getVkDevice(), vk_buffer->vk_memory, offset, size, 0, &mapped));
		AKA_ASSERT(mapped != nullptr, "Failed to map memory");
		memcpy(mapped, data, static_cast<size_t>(size));
		vkUnmapMemory(getVkDevice(), vk_buffer->vk_memory);
	}
	else if (VulkanBuffer::isTransferable(vk_buffer->usage))
	{
		VkBuffer vk_stagingBuffer = VulkanBuffer::createVkBuffer(getVkDevice(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		VkDeviceMemory vk_stagingMemory = VulkanBuffer::createVkDeviceMemory(getVkDevice(), getVkPhysicalDevice(), vk_stagingBuffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		
		void* mapped = nullptr;
		VK_CHECK_RESULT(vkMapMemory(getVkDevice(), vk_stagingMemory, 0, size, 0, &mapped));
		AKA_ASSERT(mapped != nullptr, "Failed to map memory");
		memcpy(mapped, data, static_cast<size_t>(size));
		vkUnmapMemory(getVkDevice(), vk_stagingMemory);

		VkCommandBuffer cmd = VulkanCommandList::createSingleTime("UploadingBuffer", getVkDevice(), getVkCommandPool(QueueType::Graphic));
		VkBufferCopy region{};
		region.srcOffset = 0;
		region.dstOffset = offset;
		region.size = size;
		vkCmdCopyBuffer(cmd, vk_stagingBuffer, vk_buffer->vk_buffer, 1, &region);
		VulkanCommandList::endSingleTime(getVkDevice(), getVkCommandPool(QueueType::Graphic), cmd, getVkQueue(QueueType::Graphic));

		vkFreeMemory(getVkDevice(), vk_stagingMemory, nullptr);
		vkDestroyBuffer(getVkDevice(), vk_stagingBuffer, nullptr);
	}
	else
	{
		throw std::runtime_error("Cannot upload to an immutable buffer");
	}
}

void VulkanGraphicDevice::download(BufferHandle buffer, void* data, uint32_t offset, uint32_t size)
{
	VulkanBuffer* vk_buffer = getVk<VulkanBuffer>(buffer);
	if (VulkanBuffer::isMappable(vk_buffer->usage))
	{
		void* mapped = nullptr;
		VK_CHECK_RESULT(vkMapMemory(getVkDevice(), vk_buffer->vk_memory, offset, size, 0, &mapped));
		AKA_ASSERT(mapped != nullptr, "Failed to map memory");
		memcpy(data, mapped, static_cast<size_t>(size));
		vkUnmapMemory(getVkDevice(), vk_buffer->vk_memory);
	}
	else if (VulkanBuffer::isTransferable(vk_buffer->usage))
	{
		VkBuffer vk_stagingBuffer = VulkanBuffer::createVkBuffer(getVkDevice(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		VkDeviceMemory vk_stagingMemory = VulkanBuffer::createVkDeviceMemory(getVkDevice(), getVkPhysicalDevice(), vk_stagingBuffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		
		VkCommandBuffer cmd = VulkanCommandList::createSingleTime("Downloading buffer", getVkDevice(), getVkCommandPool(QueueType::Graphic));
		VkBufferCopy region{};
		region.srcOffset = offset;
		region.dstOffset = 0;
		region.size = size;// VK_WHOLE_SIZE ?
		vkCmdCopyBuffer(cmd, vk_stagingBuffer, vk_buffer->vk_buffer, 1, &region);
		VulkanCommandList::endSingleTime(getVkDevice(), getVkCommandPool(QueueType::Graphic), cmd, getVkQueue(QueueType::Graphic));

		void* mapped = nullptr;
		VK_CHECK_RESULT(vkMapMemory(getVkDevice(), vk_stagingMemory, 0, size, 0, &mapped));
		AKA_ASSERT(mapped != nullptr, "Failed to map memory");
		memcpy(mapped, data, static_cast<size_t>(size));
		vkUnmapMemory(getVkDevice(), vk_stagingMemory);

		vkFreeMemory(getVkDevice(), vk_stagingMemory, nullptr);
		vkDestroyBuffer(getVkDevice(), vk_stagingBuffer, nullptr);
	}
	else
	{
		throw std::runtime_error("Cannot download from an immutable buffer");
	}
}

void* VulkanGraphicDevice::map(BufferHandle buffer, BufferMap map)
{
	VulkanBuffer* vk_buffer = getVk<VulkanBuffer>(buffer);
	AKA_ASSERT(VulkanBuffer::isMappable(vk_buffer->usage), "Cannot map this buffer.");
	void* data = nullptr;
	VK_CHECK_RESULT(vkMapMemory(getVkDevice(), vk_buffer->vk_memory, 0, vk_buffer->size, 0, &data));
	return data;
}

void VulkanGraphicDevice::unmap(BufferHandle buffer)
{
	VulkanBuffer* vk_buffer = getVk<VulkanBuffer>(buffer);
	AKA_ASSERT(VulkanBuffer::isMappable(vk_buffer->usage), "Cannot map this buffer.");
	vkUnmapMemory(getVkDevice(), vk_buffer->vk_memory);
}

void VulkanGraphicDevice::destroy(BufferHandle buffer)
{
	if (get(buffer) == nullptr) return;

	VulkanBuffer* vk_buffer = getVk<VulkanBuffer>(buffer);
	vk_buffer->destroy(this);
	m_bufferPool.release(vk_buffer);
}

const Buffer* VulkanGraphicDevice::get(BufferHandle buffer)
{
	return static_cast<const Buffer*>(buffer.__data);
}

};
};