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
	VulkanFrame();
	VulkanFrame(const char* name);

	void create(VulkanGraphicDevice* device);
	void destroy(VulkanGraphicDevice* device);

	void wait(VkDevice device);
	VulkanCommandList* allocateCommand(VulkanGraphicDevice* device, QueueType queue);
	void releaseCommand(VulkanCommandList* commandList);

	VkSemaphore presentSemaphore[EnumCount<QueueType>()];
	VkSemaphore acquireSemaphore;
	VkFence presentFence[EnumCount<QueueType>()];

	VulkanCommandList mainCommandLists[EnumCount<QueueType>()];

	VkCommandPool commandPool[EnumCount<QueueType>()];
	Vector<VulkanCommandList> commandLists[EnumCount<QueueType>()];
};


struct BackBufferTextures
{
	TextureHandle color;
};


class VulkanGraphicDevice;

class VulkanSwapchain : 
	EventListener<BackbufferResizeEvent>
{
public:
	VulkanSwapchain();

	bool initialize(VulkanGraphicDevice* device, PlatformDevice* platform);
	void shutdown(VulkanGraphicDevice* device);

	void onReceive(const BackbufferResizeEvent& e) override;
	void recreate(VulkanGraphicDevice* device);

	VulkanFrame* acquireNextImage(VulkanGraphicDevice* context);
	SwapchainStatus present(VulkanGraphicDevice* device, VulkanFrame& frame);

	BackbufferHandle createBackbuffer(VulkanGraphicDevice* device, RenderPassHandle handle, const Attachment* _additionalAttachments, uint32_t _count, const Attachment* _depthAttachment);
	Backbuffer* getBackbuffer(VulkanGraphicDevice* device, BackbufferHandle handle);
	void destroyBackbuffer(VulkanGraphicDevice* device, BackbufferHandle handle);

	uint32_t getImageCount() const { return m_imageCount; }
	FrameIndex getCurrentFrameIndex() const { return m_currentFrameIndex; }
	TextureFormat getColorFormat() const { return m_colorFormat; }
	TextureFormat getDepthFormat() const { return m_depthFormat; }

	VulkanFrame& getVkFrame(FrameHandle handle);
	FrameIndex getVkFrameIndex(FrameHandle handle);
	VulkanFrame& getVkFrame(FrameIndex index) { return m_frames[index.value()]; }
	const VulkanFrame& getVkFrame(FrameIndex index) const { return m_frames[index.value()]; }

	uint32_t width() const { return m_width; }
	uint32_t height() const { return m_height; }
private:
	void createSwapchain(VulkanGraphicDevice* _device, PlatformDevice* _platform, VkSwapchainKHR _oldSwapchain);
	void createImageViews(VulkanGraphicDevice* _device);
	void createFrames(VulkanGraphicDevice* _device);

	void destroySwapchain(VulkanGraphicDevice* _device);
	void destroyImageViews(VulkanGraphicDevice* _device);
	void destroyFrames(VulkanGraphicDevice* _device);
	void destroyFramebuffers(VulkanGraphicDevice* _device);

	void recreateFramebuffers(VulkanGraphicDevice* _device);
private:
	bool m_needRecreation;
	uint32_t m_width, m_height;
	VkFormat m_surfaceFormat;
	PlatformDevice* m_platform;
	VkSwapchainKHR m_swapchain;
	//VkSurfaceKHR m_surface;
	uint32_t m_imageCount;
	FrameIndex m_currentFrameIndex;
	VulkanFrame m_frames[MaxFrameInFlight];

	std::unordered_map<BackbufferHandle, Backbuffer> m_backbuffers;
	TextureFormat m_colorFormat;
	TextureFormat m_depthFormat;
	Vector<BackBufferTextures> m_backbufferTextures;
};

};
};