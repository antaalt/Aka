#pragma once

#include "VulkanContext.h"
#include <Aka/Core/Container/Vector.h>
#include <Aka/Core/Event.h>

#include "VulkanFramebuffer.h"

namespace aka {

struct VulkanFrame : Frame
{
	VkSemaphore acquireSemaphore;
	VkSemaphore presentSemaphore;
	VkFence acquireFence;
	VkFence presentFence;

	void wait(VkDevice device);
};

class VulkanGraphicDevice;

struct VulkanSwapchain : 
	EventListener<BackbufferResizeEvent>
{
	void initialize(VulkanGraphicDevice* device, PlatformDevice* platform);
	void shutdown(VulkanGraphicDevice* device);

	void onReceive(const BackbufferResizeEvent& e) override;
	void recreate(VulkanGraphicDevice* context);

	VulkanFrame* acquireNextImage(VulkanGraphicDevice* context);
	void present(VulkanGraphicDevice* context, VulkanFrame* frame);

	bool needRecreation;
	PlatformDevice* platform;
	VkSwapchainKHR swapchain;
	//VkSurfaceKHR surface;
	uint32_t imageCount;
	FrameIndex currentFrameIndex;
	VulkanFrame frames[FrameIndex::MaxInFlight];

	std::vector<VulkanFramebuffer*> backbuffers;

};

};