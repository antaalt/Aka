#pragma once

#include "VulkanContext.h"
#include <Aka/Core/Container/Vector.h>
#include <Aka/Graphic/Swapchain.h>
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
	VulkanCommandEncoder* allocateCommand(VulkanGraphicDevice* device, QueueType queue);
	void releaseCommand(VulkanCommandEncoder* commandList);

	VkSemaphore presentSemaphore[EnumCount<QueueType>()];
	VkSemaphore acquireSemaphore;
	VkFence presentFence[EnumCount<QueueType>()];

	VulkanCommandEncoder* mainCommandEncoders[EnumCount<QueueType>()];

	VkCommandPool commandPool[EnumCount<QueueType>()];
	Vector<VulkanCommandEncoder*> commandEncoders[EnumCount<QueueType>()];
};

class VulkanSurface : public Surface
{
public:
	VulkanSurface(const char* name, PlatformWindow* window);

	void create(VulkanGraphicDevice* device);
	void destroy(VulkanGraphicDevice* device);

	static VkSurfaceKHR createSurface(VulkanGraphicDevice* device, PlatformWindow* platform);

	VkSurfaceKHR vk_surface;
};


struct BackBufferTextures
{
	TextureHandle color;
};


class VulkanGraphicDevice;

class VulkanSwapchain : 
	public Swapchain,
	EventListener<BackbufferResizeEvent>
{
public:
	VulkanSwapchain(const char* name, SurfaceHandle surface, uint32_t width, uint32_t height, TextureFormat format, SwapchainMode mode, SwapchainType type);

	bool initialize(VulkanGraphicDevice* device);
	void shutdown(VulkanGraphicDevice* device);

	void onReceive(const BackbufferResizeEvent& e) override;
	void recreate(VulkanGraphicDevice* device, uint32_t width, uint32_t height, TextureFormat format, SwapchainMode mode, SwapchainType type);

	VulkanFrame* acquireNextImage(VulkanGraphicDevice* context);
	SwapchainStatus present(VulkanGraphicDevice* device, VulkanFrame& frame);

	BackbufferHandle createBackbuffer(VulkanGraphicDevice* device, RenderPassHandle handle, const Attachment* _additionalAttachments, uint32_t _count, const Attachment* _depthAttachment);
	Backbuffer* getBackbuffer(VulkanGraphicDevice* device, BackbufferHandle handle);
	void destroyBackbuffer(VulkanGraphicDevice* device, BackbufferHandle handle);

	FrameIndex getCurrentFrameIndex() const { return m_currentFrameIndex; }

	VulkanFrame& getVkFrame(FrameHandle handle);
	FrameIndex getVkFrameIndex(FrameHandle handle);
	VulkanFrame& getVkFrame(FrameIndex index) { return m_frames[index.value()]; }
	const VulkanFrame& getVkFrame(FrameIndex index) const { return m_frames[index.value()]; }
private:
	void createSwapchain(VulkanGraphicDevice* _device, VkSwapchainKHR _oldSwapchain);
	void createImageViews(VulkanGraphicDevice* _device);
	void createFrames(VulkanGraphicDevice* _device);

	void destroySwapchain(VulkanGraphicDevice* _device);
	void destroyImageViews(VulkanGraphicDevice* _device);
	void destroyFrames(VulkanGraphicDevice* _device);
	void destroyFramebuffers(VulkanGraphicDevice* _device);

	void recreateFramebuffers(VulkanGraphicDevice* _device);
private:
	bool m_needRecreation;
	VkFormat m_surfaceFormat;
	VkSwapchainKHR m_swapchain;
	FrameIndex m_currentFrameIndex;
	VulkanFrame m_frames[MaxFrameInFlight];

	std::unordered_map<BackbufferHandle, Backbuffer> m_backbuffers;
	Vector<BackBufferTextures> m_backbufferTextures;
};

};
};