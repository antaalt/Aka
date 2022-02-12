#pragma once

#include "VulkanContext.h"
#include <Aka/Core/Container/Vector.h>

#include "VulkanFramebuffer.h"

namespace aka {

struct VulkanFrame : Frame
{
	bool needRecreation;
	VkSemaphore acquireSemaphore;
	VkSemaphore presentSemaphore;
	VkFence acquireFence;
	VkFence presentFence;

	void wait(VkDevice device);
};

class VulkanGraphicDevice;

struct VulkanSwapchain
{
	void initialize(VulkanGraphicDevice* device, PlatformDevice* platform);
	void shutdown(VulkanGraphicDevice* device);

	VulkanFrame* acquireNextImage(VulkanContext* context);
	void present(VulkanContext* context, VulkanFrame* frame);

	VkSwapchainKHR swapchain;
	//VkSurfaceKHR surface;
	uint32_t imageCount;
	FrameIndex currentFrameIndex;
	VulkanFrame frames[FrameIndex::MaxInFlight];

	std::vector<VulkanFramebuffer*> backbuffers;

};

};