#pragma once

#include "VulkanContext.h"
#include <Aka/Core/Container/Vector.h>
#include <Aka/Core/Event.h>

#include "VulkanFramebuffer.h"
#include "VulkanCommandList.h"

namespace aka {
namespace gfx {

struct VulkanFrame : Frame
{
	friend class VulkanGraphicDevice;
	friend class VulkanSwapchain;

	static constexpr uint32_t SemaphoreCount = EnumCount<QueueType>() + 1;

	VkSemaphore semaphore[SemaphoreCount];
	VkFence presentFence[EnumCount<QueueType>()];

	VulkanCommandList commandLists[EnumCount<QueueType>()];

	void wait(VkDevice device);
};


struct BackBufferTextures
{
	TextureHandle color;
	TextureHandle depth; // TODO should not be here...
};


class VulkanGraphicDevice;

class VulkanSwapchain : 
	EventListener<BackbufferResizeEvent>
{
public:
	VulkanSwapchain();

	void initialize(VulkanGraphicDevice* device, PlatformDevice* platform);
	void shutdown(VulkanGraphicDevice* device);

	void onReceive(const BackbufferResizeEvent& e) override;
	void recreate(VulkanGraphicDevice* device);

	VulkanFrame* acquireNextImage(VulkanGraphicDevice* context);
	SwapchainStatus present(VulkanGraphicDevice* device, VulkanFrame* frame);

	BackbufferHandle createBackbuffer(VulkanGraphicDevice* device, RenderPassHandle handle);
	void destroyBackbuffer(VulkanGraphicDevice* device, BackbufferHandle handle);

	uint32_t getImageCount() const { return m_imageCount; }
	FrameIndex getCurrentFrameIndex() const { return m_currentFrameIndex; }
	TextureFormat getColorFormat() const { return m_colorFormat; }
	TextureFormat getDepthFormat() const { return m_depthFormat; }

	// This is frame related, not swapchain.
	/*VkFence getVkPresentFence(FrameIndex index) { return m_frames[index.value].presentFence; }
	VkFence getVkAcquireFence(FrameIndex index) { return m_frames[index.value].acquireFence; }
	VkSemaphore getVkAcquireSemaphore(FrameIndex index) { return m_frames[index.value].acquireSemaphore; }
	VkSemaphore getVkPesentSemaphore(FrameIndex index) { return m_frames[index.value].presentSemaphore; }*/
	const VulkanFrame& getVkFrame(FrameIndex index) const { return m_frames[index.value]; }
private:
	bool m_needRecreation;
	uint32_t m_width, m_height;
	PlatformDevice* m_platform;
	VkSwapchainKHR m_swapchain;
	//VkSurfaceKHR m_surface;
	uint32_t m_imageCount;
	FrameIndex m_currentFrameIndex;
	VulkanFrame m_frames[FrameIndex::MaxInFlight];

	std::unordered_map<RenderPassState, Backbuffer> m_backbuffers;
	TextureFormat m_colorFormat;
	TextureFormat m_depthFormat;
	std::vector<BackBufferTextures> m_backbufferTextures;
};

};
};