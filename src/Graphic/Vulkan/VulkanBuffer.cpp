#include "VulkanBuffer.h"
#include "VulkanGraphicDevice.h"
#include "VulkanBarrier.h"

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
	vkFreeMemory(device->getVkDevice(), vk_memory, getVkAllocator());
	vkDestroyBuffer(device->getVkDevice(), vk_buffer, getVkAllocator());
	vk_buffer = VK_NULL_HANDLE;
	vk_memory = VK_NULL_HANDLE;
}

void VulkanBuffer::copyFrom(VkCommandBuffer cmd, VulkanBuffer* src, uint32_t srcOffset, uint32_t dstOffset, uint32_t range)
{
	AKA_ASSERT((range == ~0U) ? size == src->size : range <= size, "Invalid sizes");

	VkBufferCopy region{};
	region.srcOffset = srcOffset;
	region.dstOffset = dstOffset;
	region.size = (range == ~0U) ? src->size : range; // VK_WHOLE_SIZE
	vkCmdCopyBuffer(cmd, src->vk_buffer, vk_buffer, 1, &region);
}

VkBuffer VulkanBuffer::createVkBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Ownership must be explicitely transfered

	VkBuffer vk_buffer = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, getVkAllocator(), &vk_buffer));
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
	VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, getVkAllocator(), &vk_memory));

	VK_CHECK_RESULT(vkBindBufferMemory(device, vk_buffer, vk_memory, 0));
	return vk_memory;
}

void VulkanGraphicDevice::transition(BufferHandle buffer, ResourceAccessType src, ResourceAccessType dst)
{
	execute("Transition resource", [=](CommandList& cmd) {
		cmd.transition(buffer, src, dst);
	}, QueueType::Graphic, false); // Blocking
}
void VulkanGraphicDevice::transfer(BufferHandle buffer, QueueType srcQueue, QueueType dstQueue, ResourceAccessType src, ResourceAccessType dst) 
{
	VulkanBuffer* vk_buffer = getVk<VulkanBuffer>(buffer);
	VulkanBuffer::transferBuffer(
		this, 
		srcQueue, dstQueue,
		vk_buffer->vk_buffer,
		src,
		dst
	);
}

void VulkanBuffer::transitionBuffer(
	VkCommandBuffer cmd,
	QueueType queueType,
	VkBuffer buffer,
	size_t size,
	size_t offset,
	ResourceAccessType oldAccess,
	ResourceAccessType newAccess
)
{
	std::pair<VulkanPipelineStageAccess, VulkanPipelineStageAccess> pipelineStageAccess = getBufferPipelineStageAccess(queueType, oldAccess, newAccess);
	VulkanBuffer::insertMemoryBarrier(
		cmd,
		buffer,
		size,
		offset,
		pipelineStageAccess.first.stage,
		pipelineStageAccess.second.stage,
		pipelineStageAccess.first.access,
		pipelineStageAccess.second.access,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED
	);
}
void VulkanBuffer::insertMemoryBarrier(VkCommandBuffer command, VkBuffer buffer, size_t size, size_t offset, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkAccessFlags srcAccess, VkAccessFlags dstAccess, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex)
{
	VkBufferMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.buffer = buffer;
	barrier.offset = offset;
	barrier.size = size;
	barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
	barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
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
void VulkanBuffer::transferBuffer(
	VulkanGraphicDevice* device,
	QueueType srcQueueType,
	QueueType dstQueueType,
	VkBuffer buffer,
	ResourceAccessType oldAccess,
	ResourceAccessType newAccess
)
{
	AKA_ASSERT(srcQueueType != dstQueueType, "Transfering buffer on same queue...");
	// Create sync 
	FenceHandle fence = device->createFence("TransferFence", 0);
	FenceValue syncValue = 42;
	// Create first sync
	CommandEncoder* srcEncoder = device->acquireCommandEncoder(srcQueueType);
	srcEncoder->record([=](CommandList& cmd) {
		VulkanCommandList& vk_command = dynamic_cast<VulkanCommandList&>(cmd);
		std::pair<VulkanPipelineStageAccess, VulkanPipelineStageAccess> pipelineStageAccess = getBufferPipelineStageAccess(srcQueueType, oldAccess, ResourceAccessType::Undefined);
		VulkanBuffer::insertMemoryBarrier(
			vk_command.getVkCommandBuffer(),
			buffer,
			VK_WHOLE_SIZE, // TODO: Transfer whole buffer, can we transfer partial ?
			0,
			pipelineStageAccess.first.stage,
			pipelineStageAccess.second.stage,
			pipelineStageAccess.first.access,
			pipelineStageAccess.second.access,
			device->getVkQueueIndex(srcQueueType),
			device->getVkQueueIndex(dstQueueType)
		);
	});
	device->submit(srcEncoder, fence, InvalidFenceValue, syncValue);
	
	CommandEncoder* dstEncoder = device->acquireCommandEncoder(dstQueueType);
	dstEncoder->record([=](CommandList& cmd) {
		VulkanCommandList& vk_command = dynamic_cast<VulkanCommandList&>(cmd);
		std::pair<VulkanPipelineStageAccess, VulkanPipelineStageAccess> pipelineStageAccess = getBufferPipelineStageAccess(dstQueueType, ResourceAccessType::Undefined, newAccess);
		VulkanBuffer::insertMemoryBarrier(
			vk_command.getVkCommandBuffer(),
			buffer,
			VK_WHOLE_SIZE, // TODO: Transfer whole buffer, can we transfer partial ?
			0,
			pipelineStageAccess.first.stage, // BOTTOM OF PIPE ?
			pipelineStageAccess.second.stage,
			pipelineStageAccess.first.access,
			pipelineStageAccess.second.access,
			device->getVkQueueIndex(srcQueueType),
			device->getVkQueueIndex(dstQueueType)
		);
	});
	device->submit(dstEncoder, fence, syncValue);
	device->wait(dstQueueType); // This should not be blocking ideally. Return a fence ?
	device->release(srcEncoder);
	device->release(dstEncoder);

	device->destroy(fence);
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

void VulkanGraphicDevice::upload(BufferHandle buffer, const void* data, size_t offset, size_t size)
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
		// Use shared memory heap if small enough.
		VkBuffer vk_stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory vk_stagingMemory = VK_NULL_HANDLE;
		if (size > m_stagingUploadHeapSize)
		{
			Logger::warn("Uploading some big boy to buffer ", vk_buffer->name,", allocating ", size, " staging bytes for upload.");
			vk_stagingBuffer = VulkanBuffer::createVkBuffer(getVkDevice(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			vk_stagingMemory = VulkanBuffer::createVkDeviceMemory(getVkDevice(), getVkPhysicalDevice(), vk_stagingBuffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		}
		else
		{
			vk_stagingBuffer = m_stagingUploadBuffer;
			vk_stagingMemory = m_stagingUploadMemory;
		}
		
		void* mapped = nullptr;
		VK_CHECK_RESULT(vkMapMemory(getVkDevice(), vk_stagingMemory, 0, size, 0, &mapped));
		AKA_ASSERT(mapped != nullptr, "Failed to map memory");
		memcpy(mapped, data, static_cast<size_t>(size));
		vkUnmapMemory(getVkDevice(), vk_stagingMemory);

		String debugName = String::format("UploadStagingBufferTo%s", vk_buffer->name);
		executeVk(debugName.cstr(), [=](VulkanCommandList& cmd) {
			VkBufferCopy region{};
			region.srcOffset = 0;
			region.dstOffset = offset;
			region.size = size;
			vkCmdCopyBuffer(cmd.getVkCommandBuffer(), vk_stagingBuffer, vk_buffer->vk_buffer, 1, &region);
		}, QueueType::Copy, false);  // Blocking

		if (vk_stagingBuffer != m_stagingUploadBuffer)
		{
			vkFreeMemory(getVkDevice(), vk_stagingMemory, getVkAllocator());
			vkDestroyBuffer(getVkDevice(), vk_stagingBuffer, getVkAllocator());
		}
	}
	else
	{
		AKA_ASSERT(false, "Cannot upload to an immutable buffer");
	}
}

void VulkanGraphicDevice::download(BufferHandle buffer, void* data, size_t offset, size_t size)
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
		// Use shared memory heap if small enough.
		VkBuffer vk_stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory vk_stagingMemory = VK_NULL_HANDLE;
		if (size > m_stagingDownloadHeapSize)
		{
			Logger::warn("Downloading some big boy from buffer ", vk_buffer->name, ", allocating ", size, " staging bytes for download.");
			vk_stagingBuffer = VulkanBuffer::createVkBuffer(getVkDevice(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			vk_stagingMemory = VulkanBuffer::createVkDeviceMemory(getVkDevice(), getVkPhysicalDevice(), vk_stagingBuffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		}
		else
		{
			vk_stagingBuffer = m_stagingDownloadBuffer;
			vk_stagingMemory = m_stagingDownloadMemory;
		}
		String debugName = String::format("DownloadBuffer%sToStaging", vk_buffer->name);
		executeVk(debugName.cstr(), [=](VulkanCommandList& cmd) {
			VkBufferCopy region{};
			region.srcOffset = offset;
			region.dstOffset = 0;
			region.size = size;// VK_WHOLE_SIZE ?
			vkCmdCopyBuffer(cmd.getVkCommandBuffer(), vk_buffer->vk_buffer, vk_stagingBuffer, 1, &region);
		}, QueueType::Copy, false); // Blocking

		void* mapped = nullptr;
		VK_CHECK_RESULT(vkMapMemory(getVkDevice(), vk_stagingMemory, 0, size, 0, &mapped));
		AKA_ASSERT(mapped != nullptr, "Failed to map memory");
		memcpy(data, mapped, static_cast<size_t>(size));
		vkUnmapMemory(getVkDevice(), vk_stagingMemory);

		if (vk_stagingBuffer != m_stagingDownloadBuffer)
		{
			vkFreeMemory(getVkDevice(), vk_stagingMemory, getVkAllocator());
			vkDestroyBuffer(getVkDevice(), vk_stagingBuffer, getVkAllocator());
		}
	}
	else
	{
		AKA_ASSERT(false, "Cannot download from an immutable buffer");
	}
}

void gfx::VulkanGraphicDevice::copy(BufferHandle src, BufferHandle dst)
{
	VulkanBuffer* vk_srcBuffer = getVk<VulkanBuffer>(src);
	VulkanBuffer* vk_dstBuffer = getVk<VulkanBuffer>(dst);
	AKA_ASSERT(vk_srcBuffer->size == vk_dstBuffer->size, "");
	String debugName = String::format("CopyBuffer%sTo%s", vk_srcBuffer->name, vk_dstBuffer->name);
	executeVk(debugName.cstr(), [=](VulkanCommandList& cmd) {
		vk_dstBuffer->copyFrom(cmd.getVkCommandBuffer(), vk_srcBuffer);
	}, QueueType::Copy, false); // Blocking
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