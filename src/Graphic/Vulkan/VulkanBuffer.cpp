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

void VulkanBuffer::copyFrom(VkCommandBuffer cmd, VulkanBuffer* src, uint32_t srcOffset, uint32_t dstOffset, uint32_t range)
{
	AKA_ASSERT(size == src->size, "Invalid sizes");

	VkBufferCopy region{};
	region.srcOffset = srcOffset;
	region.dstOffset = dstOffset;
	region.size = (range == ~0U) ? src->size : range;
	vkCmdCopyBuffer(cmd, src->vk_buffer, vk_buffer, 1, &region);
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


VkPipelineStageFlags pipelineStageForLayout(ResourceAccessType type, bool src)
{
	// TODO for buffers also
	// VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
	// VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT
	switch (type)
	{
	default:
		AKA_ASSERT(false, "Invalid access type");
		[[fallthrough]];
	case ResourceAccessType::Undefined:
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	case ResourceAccessType::Resource:
		return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
			| VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
			| VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
#if 0 // Tesselation support...
			| VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
			| VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
#endif
			;
	case ResourceAccessType::Attachment:
		AKA_ASSERT(false, "Weird");
		return VK_PIPELINE_STAGE_NONE; // Cant have buffer as attachment
	case ResourceAccessType::Storage:
		return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
			| VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
			| VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
#if 0 // Tesselation support...
			| VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
			| VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
#endif
			;
	case ResourceAccessType::CopySRC:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	case ResourceAccessType::CopyDST:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	case ResourceAccessType::Present:
		AKA_ASSERT(false, "Weird");
		return VK_PIPELINE_STAGE_NONE; // Cant present a buffer
	}
}

// Check VulkanTexture::accessFlagForLayout for more info
VkAccessFlags accessFlagForLayout(ResourceAccessType type, bool src)
{
	switch (type)
	{
	default:
		AKA_ASSERT(false, "Invalid access type");
		[[fallthrough]];
	case ResourceAccessType::Undefined:
#if defined(VK_VERSION_1_3)
		return VK_ACCESS_NONE;
#else
		return 0;
#endif
	case ResourceAccessType::Resource: // Read only
		return VK_ACCESS_SHADER_READ_BIT; // Only for shader read
		//return VK_ACCESS_MEMORY_READ_BIT; // Most general purpose
		//return VK_ACCESS_HOST_READ_BIT; // access on host (VK_PIPELINE_STAGE_HOST_BIT)
	case ResourceAccessType::Attachment:
		AKA_ASSERT(false, "Weird");
		return VK_ACCESS_NONE;
	case ResourceAccessType::Storage:
		return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	case ResourceAccessType::CopySRC:
		return VK_ACCESS_TRANSFER_READ_BIT; // VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT 
	case ResourceAccessType::CopyDST:
		return VK_ACCESS_TRANSFER_WRITE_BIT; // VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT 
	case ResourceAccessType::Present:
		AKA_ASSERT(false, "Weird");
		return VK_ACCESS_NONE;
	}
}

void VulkanBuffer::transitionBuffer(
	VkCommandBuffer cmd,
	VkBuffer buffer,
	size_t size,
	size_t offset,
	ResourceAccessType oldAccess,
	ResourceAccessType newAccess
)
{
	VulkanBuffer::insertMemoryBarrier(
		cmd,
		buffer,
		size,
		offset,
		accessFlagForLayout(oldAccess, true),
		accessFlagForLayout(newAccess, false),
		pipelineStageForLayout(oldAccess, true),
		pipelineStageForLayout(newAccess, true)
	);
}
void VulkanBuffer::insertMemoryBarrier(VkCommandBuffer command, VkBuffer buffer, size_t size, size_t offset, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
{
	VkBufferMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
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
		1, &barrier,
		0, nullptr
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

void gfx::VulkanGraphicDevice::copy(BufferHandle src, BufferHandle dst)
{
	VulkanBuffer* vk_srcBuffer = getVk<VulkanBuffer>(src);
	VulkanBuffer* vk_dstBuffer = getVk<VulkanBuffer>(dst);
	AKA_ASSERT(vk_srcBuffer->size == vk_dstBuffer->size, "");

	VkCommandBuffer cmd = VulkanCommandList::createSingleTime("Copying buffer", getVkDevice(), getVkCommandPool(QueueType::Graphic));
	vk_dstBuffer->copyFrom(cmd, vk_srcBuffer);
	VulkanCommandList::endSingleTime(getVkDevice(), getVkCommandPool(QueueType::Graphic), cmd, getVkQueue(QueueType::Graphic));
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