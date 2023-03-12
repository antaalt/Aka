#pragma once

#include "VulkanContext.h"
#include <Aka/Core/Container/Vector.h>
#include <Aka/Core/Event.h>

#include "VulkanFramebuffer.h"

namespace aka {
namespace gfx {

struct VulkanFrame : Frame
{
	VkSemaphore acquireSemaphore;
	VkSemaphore presentSemaphore;
	VkFence acquireFence;
	VkFence presentFence;

	void wait(VkDevice device);
};


struct BackBufferTextures
{
	TextureHandle color;
	TextureHandle depth; // TODO should not be here...
};


class VulkanGraphicDevice;

struct VulkanSwapchain : 
	EventListener<BackbufferResizeEvent>
{
	void initialize(VulkanGraphicDevice* device, PlatformDevice* platform);
	void shutdown(VulkanGraphicDevice* device);

	void onReceive(const BackbufferResizeEvent& e) override;
	void recreate(VulkanGraphicDevice* device);

	VulkanFrame* acquireNextImage(VulkanGraphicDevice* context);
	void present(VulkanGraphicDevice* device, VulkanFrame* frame);

	BackbufferHandle createBackbuffer(VulkanGraphicDevice* device, RenderPassHandle handle);

	//uint32_t getImageCount();

//private:
	bool needRecreation;
	PlatformDevice* platform;
	VkSwapchainKHR swapchain;
	//VkSurfaceKHR surface;
	uint32_t imageCount;
	FrameIndex currentFrameIndex;
	VulkanFrame frames[FrameIndex::MaxInFlight];

	std::unordered_map<RenderPassState, Backbuffer> backbuffers;

	std::vector<BackBufferTextures> backbufferTextures;
};

};
};