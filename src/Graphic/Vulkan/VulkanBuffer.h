#pragma once
#include <Aka/Graphic/GraphicDevice.h>

#if defined(AKA_USE_VULKAN)

#include "VulkanContext.h"

namespace aka {
namespace gfx {

struct VulkanBuffer : Buffer
{
	VulkanBuffer(const char* name, BufferType type, uint32_t size, BufferUsage usage, BufferCPUAccess access);

	VkBuffer vk_buffer;
	VkDeviceMemory vk_memory;

	void create(VulkanContext& context);
	void destroy(VulkanContext& context);

	static VkBuffer createVkBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage);
	static VkDeviceMemory createVkDeviceMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer buffer, VkMemoryPropertyFlags properties);
};

};
};

#endif