#pragma once
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Graphic/GraphicPool.h>

#if defined(AKA_USE_VULKAN)

#include "VulkanContext.h"

namespace aka {

struct VulkanBuffer : Buffer
{
	VkBuffer vk_buffer;
	VkDeviceMemory vk_memory;

	static VkBuffer createVkBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage);
	static VkDeviceMemory createVkDeviceMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer buffer, VkMemoryPropertyFlags properties);
};

};

#endif