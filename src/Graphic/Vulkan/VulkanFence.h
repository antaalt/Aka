#pragma once

#include "VulkanContext.h"

#include <Aka/Graphic/Fence.h>

namespace aka {
namespace gfx {

class VulkanGraphicDevice;

struct VulkanFence : Fence
{
	VulkanFence(const char* name);

	// We use vulkan timeline semaphore extension as it can work both as a fence & a semaphore (similar to D3D12 fences)
	VkSemaphore vk_sempahore;

	void create(VulkanGraphicDevice* device, FenceValue value);
	void destroy(VulkanGraphicDevice* device);

	static VkSemaphore createVkTimelineSemaphore(VulkanGraphicDevice* device, FenceValue initialValue);
};

};
};