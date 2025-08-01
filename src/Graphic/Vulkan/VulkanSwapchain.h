#pragma once

#include "VulkanCommon.hpp"
#include <Aka/Core/Container/Vector.h>
#include <Aka/Graphic/Swapchain.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Core/Event.h>

#include "VulkanFramebuffer.h"
#include "VulkanCommandList.h"

namespace aka {
namespace gfx {

class VulkanInstance;

// Synchronisation are not tied to a fixed image as it will cause issue getting them back.
CREATE_STRICT_TYPE(uint32_t, ImageSyncIndex)
const ImageSyncIndex InvalidImageSyncIndex = ImageSyncIndex((ImageSyncIndex::Type)~0);

struct VulkanFrame : Frame
{
	VulkanFrame(FrameIndex frame);
	VulkanFrame(const char* name, FrameIndex frame);

	void create(VulkanGraphicDevice* device);
	void destroy(VulkanGraphicDevice* device);

	void wait(VkDevice device);
	VulkanCommandEncoder* allocateCommand(VulkanGraphicDevice* device, QueueType queue);
	void releaseCommand(VulkanCommandEncoder* commandList);

	VkFence presentFence[EnumCount<QueueType>()];

	VulkanCommandEncoder* mainCommandEncoders[EnumCount<QueueType>()];

	VkCommandPool commandPool[EnumCount<QueueType>()];
	Vector<VulkanCommandEncoder*> commandEncoders[EnumCount<QueueType>()];
};

struct VulkanImage
{
	VulkanImage(ImageIndex image, TextureHandle color);
	VulkanImage(VulkanImage&) = delete;
	VulkanImage& operator=(VulkanImage&) = delete;
	VulkanImage(VulkanImage&&) = default;
	VulkanImage& operator=(VulkanImage&&) = default;
	~VulkanImage() {}

	void create(VulkanGraphicDevice* device);
	void destroy(VulkanGraphicDevice* device);

	void setImageSyncIndex(ImageSyncIndex index) { m_syncImage = index; }
	ImageSyncIndex getImageSyncIndex() const { return m_syncImage; }
	ImageIndex getImageIndex() const { return m_image; }
	TextureHandle getTexture() const { return m_color; }
protected:
	ImageIndex m_image;
	TextureHandle m_color; // Owned by swapchain.
	ImageSyncIndex m_syncImage = InvalidImageSyncIndex; // Will change every frame depending on available sync data.
};

struct VulkanImageSynchronisation
{
	VulkanImageSynchronisation() {}
	// NTM C++, Rust FTW
	VulkanImageSynchronisation(VulkanImageSynchronisation&) = delete;
	VulkanImageSynchronisation& operator=(VulkanImageSynchronisation&) = delete;
	VulkanImageSynchronisation(VulkanImageSynchronisation&&) = default;
	VulkanImageSynchronisation& operator=(VulkanImageSynchronisation&&) = default;
	~VulkanImageSynchronisation() {}

	void create(VulkanGraphicDevice* device);
	void destroy(VulkanGraphicDevice* device);

	VkSemaphore presentSemaphore[EnumCount<QueueType>()] = { VK_NULL_HANDLE };
	VkSemaphore acquireSemaphore = VK_NULL_HANDLE;
};

class VulkanSurface : public Surface
{
public:
	VulkanSurface(const char* name, PlatformWindow* window);

	void create(VulkanInstance* instance);
	void destroy(VulkanInstance* instance);

	static VkSurfaceKHR createSurface(VulkanInstance* instance, PlatformWindow* platform);

	VkSurfaceKHR vk_surface;
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
	VulkanImage& getVkImage(ImageIndex index);
	VulkanImageSynchronisation& getVkImageSync(ImageSyncIndex index);
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
	ImageSyncIndex m_currentImageSyncIndex;
	Vector<VulkanImage> m_images;
	Vector<VulkanImageSynchronisation> m_imageSync;
};

};
};