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

	static VkBuffer createVkBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage);
	static VkDeviceMemory createVkDeviceMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer buffer, VkMemoryPropertyFlags properties);
};

};
};

#endif