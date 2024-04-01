#pragma once
#include <Aka/Graphic/GraphicDevice.h>

#if defined(AKA_USE_VULKAN)

#include "VulkanContext.h"

namespace aka {
namespace gfx {

class VulkanGraphicDevice;

struct VulkanBuffer : Buffer
{
	VulkanBuffer(const char* name, BufferType type, uint32_t size, BufferUsage usage, BufferCPUAccess access);

	VkBuffer vk_buffer;
	VkDeviceMemory vk_memory;

	void create(VulkanGraphicDevice* context);
	void destroy(VulkanGraphicDevice* context);

	void copyFrom(VkCommandBuffer cmd, VulkanBuffer* src, uint32_t srcOffset = 0, uint32_t dstOffset = 0, uint32_t range = ~0U);

	static VkBuffer createVkBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage);
	static VkDeviceMemory createVkDeviceMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer buffer, VkMemoryPropertyFlags properties);
	
	static void transitionBuffer(VkCommandBuffer cmd, VkBuffer buffer, size_t size, size_t offset, ResourceAccessType oldAccess, ResourceAccessType newAccess);
	static void insertMemoryBarrier(VkCommandBuffer command, VkBuffer buffer, size_t size, size_t offset, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);

	static bool isMappable(BufferUsage usage);
	static bool isTransferable(BufferUsage usage);
};

};
};

#endif