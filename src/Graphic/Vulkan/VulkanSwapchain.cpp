#include "VulkanSwapchain.h"

#include "VulkanGraphicDevice.h"
#include "VulkanInstance.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "../../Platform/GLFW3/PlatformGLFW3.h"

namespace aka {
namespace gfx {

VulkanSurface::VulkanSurface(const char* name, PlatformWindow* window) :
	Surface(name, window),
	vk_surface(VK_NULL_HANDLE)
{

}
void VulkanSurface::create(VulkanInstance* instance)
{
	vk_surface = createSurface(instance, window);
}
void VulkanSurface::destroy(VulkanInstance* instance)
{
	vkDestroySurfaceKHR(instance->getVkInstance(), vk_surface, getVkAllocator());
	vk_surface = VK_NULL_HANDLE;
}
VkSurfaceKHR VulkanSurface::createSurface(VulkanInstance* instance, PlatformWindow* window)
{
	VkSurfaceKHR surface;
	VK_CHECK_RESULT(glfwCreateWindowSurface(
		instance->getVkInstance(),
		reinterpret_cast<PlatformWindowGLFW3*>(window)->getGLFW3Handle(), // TODO: other platform.
		getVkAllocator(),
		&surface
	));
	return surface;
}
SwapchainHandle VulkanGraphicDevice::createSwapchain(const char* name, SurfaceHandle surface, uint32_t width, uint32_t height, TextureFormat format, SwapchainMode mode, SwapchainType type)
{
	VulkanSwapchain* vk_swapchain = m_swapchainPool.acquire(name, surface, width, height, format, mode, type);
	vk_swapchain->initialize(this);

	return SwapchainHandle{ vk_swapchain };
}
void VulkanGraphicDevice::destroy(SwapchainHandle swapchain)
{
	if (swapchain == SwapchainHandle::null) return;

	VulkanSwapchain* vk_swapchain = getVk<VulkanSwapchain>(swapchain);
	vk_swapchain->shutdown(this);

	m_swapchainPool.release(vk_swapchain);
}
const Swapchain* VulkanGraphicDevice::get(SwapchainHandle swapchain)
{
	return static_cast<const Swapchain*>(swapchain.__data);
}
SwapchainExtent VulkanGraphicDevice::getSwapchainExtent(SwapchainHandle handle)
{
	VulkanSwapchain* vk_swapchain = getVk<VulkanSwapchain>(handle);
	return SwapchainExtent{
		vk_swapchain->width,
		vk_swapchain->height
	};
}
TextureFormat VulkanGraphicDevice::getSwapchainFormat(SwapchainHandle handle)
{
	VulkanSwapchain* vk_swapchain = getVk<VulkanSwapchain>(handle);
	return vk_swapchain->format;
}
uint32_t VulkanGraphicDevice::getSwapchainImageCount(SwapchainHandle handle)
{
	VulkanSwapchain* vk_swapchain = getVk<VulkanSwapchain>(handle);
	return vk_swapchain->imageCount;
}
void VulkanGraphicDevice::resize(SwapchainHandle handle, uint32_t width, uint32_t height, TextureFormat format, SwapchainMode mode, SwapchainType type)
{
	VulkanSwapchain* vk_swapchain = getVk<VulkanSwapchain>(handle);
	vk_swapchain->recreate(this, width, height, format, mode, type);
}

VkSurfaceFormatKHR getSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	uint32_t formatCount;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr));
	Vector<VkSurfaceFormatKHR> formats;
	if (formatCount == 0)
	{
		Logger::error("No valid format for surface");
		return VkSurfaceFormatKHR{ VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}
	formats.resize(formatCount);
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data()));
	VkSurfaceFormatKHR surfaceFormat;
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		surfaceFormat = VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}
	for (const VkSurfaceFormatKHR& availableFormat : formats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			surfaceFormat = availableFormat;
			break;
		}
	}
	return formats[0];
}

bool isPresentModeAvailable(const Vector<VkPresentModeKHR>& presentModes, VkPresentModeKHR presentMode)
{
	return presentModes.end() != std::find(presentModes.begin(), presentModes.end(), presentMode);
}

VkPresentModeKHR getPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	uint32_t presentModeCount;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));

	Vector<VkPresentModeKHR> presentModes;
	if (presentModeCount == 0)
	{
		Logger::error("No valid present mode for surface");
		return VK_PRESENT_MODE_FIFO_KHR;
	}
	presentModes.resize(presentModeCount);
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data()));
	// https://www.intel.com/content/www/us/en/developer/articles/training/api-without-secrets-introduction-to-vulkan-part-2.html?language=en#_Toc445674479
	VkPresentModeKHR bestMode;
	if (isPresentModeAvailable(presentModes, VK_PRESENT_MODE_MAILBOX_KHR)) // VSync with lowest latency, no tearing.
		bestMode = VK_PRESENT_MODE_MAILBOX_KHR;
	else if (isPresentModeAvailable(presentModes, VK_PRESENT_MODE_IMMEDIATE_KHR)) // no vsync. tearing.
		bestMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	else
		bestMode = VK_PRESENT_MODE_FIFO_KHR; // VSync, always available, no tearing.

	return bestMode;
}

VkExtent2D getSurfaceExtent(VkSurfaceCapabilitiesKHR& capabilities, uint32_t w, uint32_t h)
{
	// TODO: if we are in borderless, take capabilities.maxImageExtent instead.
	// Need to move this up to App.
	// App should not store width & height & instead retrieve swapchain interface from device, & request extent struct.
	
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		return VkExtent2D{
			max(capabilities.minImageExtent.width, min(capabilities.maxImageExtent.width, w)),
			max(capabilities.minImageExtent.height, min(capabilities.maxImageExtent.height, h))
		};
	}
}

VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const Vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates) 
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
		{
			return format;
		}
	}
	return VK_FORMAT_UNDEFINED;
}

VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) {
	Vector<VkFormat> formats;
	formats.append(VK_FORMAT_D32_SFLOAT);
	formats.append(VK_FORMAT_D32_SFLOAT_S8_UINT);
	formats.append(VK_FORMAT_D24_UNORM_S8_UINT);
	return findSupportedFormat(
		physicalDevice,
		formats,
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VulkanSwapchain::VulkanSwapchain(const char* name, SurfaceHandle surface, uint32_t width, uint32_t height, TextureFormat format, SwapchainMode mode, SwapchainType type) :
	Swapchain(name, surface, width, height, format, mode, type),
	m_needRecreation(false),
	m_surfaceFormat(VK_FORMAT_UNDEFINED),
	m_swapchain(VK_NULL_HANDLE),
	m_currentFrameIndex{ 0 },
	m_frames{ VulkanFrame(FrameIndex(0)), VulkanFrame(FrameIndex(1)), VulkanFrame(FrameIndex(2))},
	m_backbuffers{},
	m_backbufferTextures{}
{
}

bool VulkanSwapchain::initialize(VulkanGraphicDevice* device)
{
	AKA_ASSERT(m_swapchain == VK_NULL_HANDLE, "Cannot call initialize with already initialized swapchain.");
	createSwapchain(device, VK_NULL_HANDLE);
	createImageViews(device);
	createFrames(device);
	return true;
}

void VulkanSwapchain::shutdown(VulkanGraphicDevice* _device)
{
	m_needRecreation = false;
	imageCount = 0;
	m_currentFrameIndex = FrameIndex(0);
	destroyFrames(_device);
	destroyFramebuffers(_device);
	destroyImageViews(_device);
	destroySwapchain(_device);
}

void VulkanSwapchain::onReceive(const BackbufferResizeEvent& e)
{
	if (e.width != this->width || e.height != this->height)
	{
		m_needRecreation = true;
	}
}

void VulkanSwapchain::recreate(VulkanGraphicDevice* _device, uint32_t width, uint32_t height, TextureFormat format, SwapchainMode mode, SwapchainType type)
{
	_device->wait();

	destroyImageViews(_device);
	VkSwapchainKHR oldSwapchain = m_swapchain;
	this->width = width;
	this->height = height;
	createSwapchain(_device, m_swapchain);
	createImageViews(_device);
	vkDestroySwapchainKHR(_device->getVkDevice(), oldSwapchain, getVkAllocator());
	recreateFramebuffers(_device);

	// Recreate pipeline to resize them if flag set.
	for (VulkanGraphicPipeline& pipeline : _device->m_graphicPipelinePool)
	{
		if (asBool(pipeline.viewport.flags & ViewportFlags::BackbufferAutoResize))
		{
			pipeline.destroy(_device);
			pipeline.create(_device);
		}
	}
}

VulkanFrame* VulkanSwapchain::acquireNextImage(VulkanGraphicDevice* device)
{
	VulkanFrame& vk_frame = m_frames[m_currentFrameIndex.value()];

	// Wait for the frame to complete before acquiring it.
	vk_frame.wait(device->getVkDevice());

	uint32_t imageIndex; // Will be initialized by following call
	VkResult result = vkAcquireNextImageKHR(
		device->getVkDevice(),
		m_swapchain,
		5 * geometry::pow(10, 9), // 1s timeout
		vk_frame.acquireSemaphore,
		VK_NULL_HANDLE,
		&imageIndex
	);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreate(device, this->width, this->height, this->format, this->mode, this->type);
		return nullptr; // Do not draw this frame.
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		AKA_ASSERT(false, "Failed to acquire swapchain image");
	}
	AKA_ASSERT(imageIndex < imageCount, "Invalid image index");
	AKA_ASSERT(MaxFrameInFlight <= imageCount, "More frames in flight than image available. May induce bugs in application.");

	// Only reset the fence if we are submitting work
	VkFence fences[EnumCount<QueueType>()] = {
		vk_frame.presentFence[EnumToIndex(QueueType::Graphic)],
		vk_frame.presentFence[EnumToIndex(QueueType::Compute)],
		vk_frame.presentFence[EnumToIndex(QueueType::Copy)],
	};
	VK_CHECK_RESULT(vkResetFences(device->getVkDevice(), EnumCount<QueueType>(), fences));

	// Set the index 
	vk_frame.setImageIndex(ImageIndex(imageIndex));

	// TODO check image finished rendering ?
	// If less image than frames in flight, might be necessary

	return &vk_frame;
}

SwapchainStatus VulkanSwapchain::present(VulkanGraphicDevice* device, VulkanFrame& vk_frame)
{
	VkSwapchainKHR swapChains[] = { m_swapchain };
	uint32_t indices[] = { vk_frame.getImageIndex().value()};

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = EnumCount<QueueType>();
	presentInfo.pWaitSemaphores = vk_frame.presentSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = indices;

	VkResult result = vkQueuePresentKHR(device->getVkPresentQueue(), &presentInfo);

	SwapchainStatus status = SwapchainStatus::Ok;
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_needRecreation)
	{
		m_needRecreation = false;
		recreate(device, this->width, this->height, this->format, this->mode, this->type);
		status = SwapchainStatus::Recreated;
	}
	else if (result != VK_SUCCESS)
	{
		Logger::error("Failed to present swap chain image!");
		status = SwapchainStatus::Error;
	}
	m_currentFrameIndex = FrameIndex((m_currentFrameIndex.value() + 1) % MaxFrameInFlight);
	return status;
}

BackbufferHandle VulkanSwapchain::createBackbuffer(VulkanGraphicDevice* device, RenderPassHandle handle, const Attachment* _additionalAttachments, uint32_t _count, const Attachment* _depthAttachment)
{
	RenderPassState state = device->get(handle)->state;
	BackbufferHandle bbhandle = BackbufferHandle{ (void*)std::hash<RenderPassState>{}(state) };
	auto it = m_backbuffers.find(bbhandle);
	if (it != m_backbuffers.end())
		return bbhandle;
	// Create a framebuffer for backbuffer compatible with given render pass.
	Backbuffer backbuffer(String::format("Backbuffer%ul", bbhandle).cstr());
	backbuffer.renderPass = handle;
	for (uint32_t i = 0; i < imageCount; i++)
	{
		Vector<Attachment> attachments;
		attachments.append(Attachment{ m_backbufferTextures[i].color, AttachmentFlag::None, 0, 0 });
		if (_additionalAttachments != nullptr)
			attachments.append(_additionalAttachments, _additionalAttachments + _count);
		FramebufferHandle fb = device->createFramebuffer("Backbuffer", handle, attachments.data(), (uint32_t)attachments.size(), _depthAttachment);
		backbuffer.handles.append(fb);
	}
	auto itInsert = m_backbuffers.insert(std::make_pair(bbhandle, std::move(backbuffer)));
	AKA_ASSERT(itInsert.second, "Failed to create backbuffer");
	return bbhandle;
}

Backbuffer* VulkanSwapchain::getBackbuffer(VulkanGraphicDevice* device, BackbufferHandle handle)
{
	auto it = m_backbuffers.find(handle);
	if (it == m_backbuffers.end())
		return nullptr;
	return &it->second;
}

void VulkanSwapchain::destroyBackbuffer(VulkanGraphicDevice* device, BackbufferHandle handle)
{
	auto it = m_backbuffers.find(handle);
	if (it == m_backbuffers.end())
		return; // Does not exist
	const Backbuffer& backbuffer = it->second;
	for (FramebufferHandle handle : backbuffer.handles)
	{
		device->destroy(handle);
	}
	m_backbuffers.erase(handle);
	// Should ref count state to do not destroy shared backbuffer.
}

void VulkanSwapchain::createSwapchain(VulkanGraphicDevice* _device, VkSwapchainKHR _oldSwapchain)
{
	VulkanSurface* vk_surface = _device->m_instance->getVk<VulkanSurface>(this->surface);
	VkSurfaceCapabilitiesKHR capabilities;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device->getVkPhysicalDevice(), vk_surface->vk_surface, &capabilities));

	VkSurfaceFormatKHR surfaceFormat = getSurfaceFormat(_device->getVkPhysicalDevice(), vk_surface->vk_surface);
	VkPresentModeKHR bestMode = getPresentMode(_device->getVkPhysicalDevice(), vk_surface->vk_surface);
	VkExtent2D extent = getSurfaceExtent(capabilities, width, height);
	imageCount = capabilities.minImageCount + 1; // Request more than the minimum to avoid driver overhead
	if (capabilities.maxImageCount == 0) // Unlimited count allowed
		capabilities.maxImageCount = 4;
	imageCount = clamp(gfx::MaxFrameInFlight + 1, capabilities.minImageCount, capabilities.maxImageCount);
	m_surfaceFormat = surfaceFormat.format;
	this->width = extent.width;
	this->height = extent.height;

	// Create swapchain
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = vk_surface->vk_surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (true)
		createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; // For blitting operations.

	HashSet<uint32_t> singleImageFamilies;
	for (QueueType queue : EnumRange<QueueType>())
		singleImageFamilies.insert(_device->getVkQueueIndex(queue));
	singleImageFamilies.insert(_device->getVkPresentQueueIndex());

	Vector<uint32_t> singleImageFamiliesData;
	for (uint32_t f : singleImageFamilies)
		singleImageFamiliesData.append(f);

	// We need concurrent mode only if graphic queue & present queue different.
	// We suppose async compute & async transfer dont deal with swapchain.
	if (_device->getVkPresentQueueIndex() != _device->getVkQueueIndex(QueueType::Graphic))
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = (uint32_t)singleImageFamilies.size();
		createInfo.pQueueFamilyIndices = singleImageFamiliesData.data();
		AKA_ASSERT(false, "Ownership transfer not implemented for swapchain.");
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = bestMode;
	createInfo.clipped = VK_TRUE; // Clip pixels hidden by another window
	createInfo.oldSwapchain = _oldSwapchain; // Old swapchain need to be destroyed elsewhere.

	VK_CHECK_RESULT(vkCreateSwapchainKHR(_device->getVkDevice(), &createInfo, getVkAllocator(), &m_swapchain));
}
void VulkanSwapchain::createImageViews(VulkanGraphicDevice* _device)
{
	VkFormat vk_depthFormat = findDepthFormat(_device->getVkPhysicalDevice());
	VkFormat vk_colorFormat = m_surfaceFormat;
	// TODO: convert vulkan format to aka format
	this->format = TextureFormat::BGRA8;
	AKA_ASSERT(vk::convert(this->format) == vk_colorFormat, "Invalid color format");

	const bool hasDepth = true;
	const bool hasStencil = false;
	// Get images & layout
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(_device->getVkDevice(), m_swapchain, &imageCount, nullptr));
	Vector<VkImage> vk_images(imageCount);
	m_backbufferTextures.resize(imageCount);
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(_device->getVkDevice(), m_swapchain, &imageCount, vk_images.data()));

	for (size_t i = 0; i < imageCount; i++)
	{
		// Create swapchain view
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = vk_images[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = vk_colorFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView view = VK_NULL_HANDLE;
		VK_CHECK_RESULT(vkCreateImageView(_device->getVkDevice(), &viewInfo, getVkAllocator(), &view));

		// Create color texture
		String str = String::format("SwapChain%u", i);
		VulkanTexture* vk_colorTexture = _device->m_texturePool.acquire(str.cstr(), this->width, this->height, 1,
			TextureType::Texture2D,
			1, 1,
			this->format,
			TextureUsage::RenderTarget
		);
		// Cannot transition color images yet as they are not acquired.
		TextureHandle colorTexture = TextureHandle{ vk_colorTexture };
		vk_colorTexture->vk_image = vk_images[i];
		vk_colorTexture->vk_view[0] = view; // Set main image view
		setDebugName(_device->getVkDevice(), vk_images[i], "SwapchainColor", i);
		setDebugName(_device->getVkDevice(), view, "SwapchainColorView", i);
		_device->executeVk("Transition backbuffer after creation", [&](VulkanCommandList& cmd) {
			VulkanTexture::transitionImageLayout(cmd.getVkCommandBuffer(), cmd.getQueueType(), vk_images[i], ResourceAccessType::Undefined, ResourceAccessType::Present, this->format);
		}, QueueType::Graphic, false);
		// No memory

		m_backbufferTextures[i].color = colorTexture;
	}
}
void VulkanSwapchain::createFrames(VulkanGraphicDevice* _device)
{
	for (VulkanFrame& frame : m_frames)
	{
		frame.create(_device);
	}
}
void VulkanSwapchain::destroySwapchain(VulkanGraphicDevice* _device)
{
	vkDestroySwapchainKHR(_device->getVkDevice(), m_swapchain, getVkAllocator());
	m_swapchain = VK_NULL_HANDLE;
}
void VulkanSwapchain::destroyImageViews(VulkanGraphicDevice* _device)
{
	for (BackBufferTextures backbuffer : m_backbufferTextures)
	{
		_device->destroy(backbuffer.color);
	}
	m_backbufferTextures.clear();
}
void VulkanSwapchain::destroyFrames(VulkanGraphicDevice* _device)
{
	for (VulkanFrame& frame : m_frames)
	{
		frame.destroy(_device);
	}
}

void VulkanSwapchain::destroyFramebuffers(VulkanGraphicDevice* _device)
{
	for (auto& backbuffer : m_backbuffers)
	{
		backbuffer.first; // TODO: Clear renderpass ref count
		for (FramebufferHandle fb : backbuffer.second.handles)
		{
			_device->destroy(fb);
		}
	}
	m_backbuffers.clear();
}

void VulkanSwapchain::recreateFramebuffers(VulkanGraphicDevice* _device)
{
	for (auto& backbufferPair : m_backbuffers)
	{
		const BackbufferHandle& bbhandle = backbufferPair.first;
		Backbuffer& backbuffer = backbufferPair.second;

		RenderPassState state = _device->get(backbuffer.renderPass)->state;
		AKA_ASSERT(bbhandle.__data == (void*)std::hash<RenderPassState>{}(state), "Invalid render pass");
		
		Vector<Vector<Attachment>> attachments(imageCount);
		Vector<Attachment> depthAttachment(imageCount, {});
		if (backbuffer.handles.size() > 0)
		{
			VulkanFramebuffer* vk_framebuffer = _device->getVk<VulkanFramebuffer>(backbuffer.handles.first());
			_device->executeVk("Recreating backbuffer", [&](VulkanCommandList& cmd) {
				for (uint32_t iAtt = 0; iAtt < vk_framebuffer->count; iAtt++)
				{
					if (state.colors[iAtt].format == TextureFormat::Swapchain)
					{
						for (uint32_t iImage = 0; iImage < imageCount; iImage++)
							attachments[iImage].append(Attachment{ m_backbufferTextures[iImage].color, AttachmentFlag::None, 0, 0 });
						continue; // Dont resize swapchain here.
					}
					else
					{
						for (uint32_t iImage = 0; iImage < imageCount; iImage++)
							attachments[iImage].append(Attachment{ vk_framebuffer->colors[iAtt].texture, AttachmentFlag::None, 0, 0 });
					}
					if (asBool(vk_framebuffer->colors[iAtt].flag & AttachmentFlag::BackbufferAutoResize))
					{
						VulkanTexture* attachment = _device->getVk<VulkanTexture>(vk_framebuffer->colors[iAtt].texture);
						attachment->destroy(_device);
						attachment->width = this->width;
						attachment->height = this->height;
						attachment->create(_device);
						// Ensure valid transitions
						ResourceAccessType finalAccessType = getInitialResourceAccessType(attachment->format, attachment->flags);
						VkImageLayout finalLayout = vk::convert(finalAccessType, attachment->format);
						VulkanTexture::transitionImageLayout(cmd.getVkCommandBuffer(),
							cmd.getQueueType(),
							attachment->vk_image,
							ResourceAccessType::Undefined,
							finalAccessType,
							attachment->format
						);
					}
				}
				if (vk_framebuffer->depth.texture != gfx::TextureHandle::null)
				{
					for (uint32_t iImage = 0; iImage < imageCount; iImage++)
						depthAttachment[iImage] = vk_framebuffer->depth;
					if (asBool(vk_framebuffer->depth.flag & AttachmentFlag::BackbufferAutoResize))
					{
						VulkanTexture* attachment = _device->getVk<VulkanTexture>(vk_framebuffer->depth.texture);
						attachment->destroy(_device);
						attachment->width = this->width;
						attachment->height = this->height;
						attachment->create(_device);
						// Ensure valid transitions
						ResourceAccessType finalAccessType = getInitialResourceAccessType(attachment->format, attachment->flags);
						VkImageLayout finalLayout = vk::convert(finalAccessType, attachment->format);
						VulkanTexture::transitionImageLayout(
							cmd.getVkCommandBuffer(),
							cmd.getQueueType(),
							attachment->vk_image,
							ResourceAccessType::Undefined,
							finalAccessType,
							attachment->format
						);
					}
				}
			}, QueueType::Graphic, false);
		}
		// Destroy previous framebuffer
		for (FramebufferHandle handle : backbuffer.handles)
		{
			_device->destroy(handle);
		}
		backbuffer.handles.clear();
		// Recreate framebuffer
		for (uint32_t i = 0; i < imageCount; i++)
		{
			Attachment* pDepthAttachment = depthAttachment[i].texture != TextureHandle::null ? &depthAttachment[i] : nullptr;
			Attachment* pAdditionalAttachments = attachments[i].data();
			FramebufferHandle fb = _device->createFramebuffer("Backbuffer", backbuffer.renderPass, pAdditionalAttachments, (uint32_t)attachments[i].size(), pDepthAttachment);
			backbuffer.handles.append(fb);
		}
	}
}

VulkanFrame& VulkanSwapchain::getVkFrame(FrameHandle handle)
{
	return *const_cast<VulkanFrame*>(reinterpret_cast<const VulkanFrame*>(handle.__data));
}

FrameIndex VulkanSwapchain::getVkFrameIndex(FrameHandle handle)
{
	VulkanFrame& frame = getVkFrame(handle);
	return frame.getFrameIndex();
}

VulkanFrame::VulkanFrame(FrameIndex frame) :
	Frame(String::format("VulkanFrame%u", frame.value()).cstr(), frame)
{
}

VulkanFrame::VulkanFrame(const char* name, FrameIndex frame) :
	Frame(name, frame)
{
}

void VulkanFrame::create(VulkanGraphicDevice* device)
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceSignaledInfo = {};
	fenceSignaledInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceSignaledInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	// Semaphores & Fences
	VK_CHECK_RESULT(vkCreateSemaphore(device->getVkDevice(), &semaphoreInfo, getVkAllocator(), &acquireSemaphore));
	setDebugName(device->getVkDevice(), acquireSemaphore, "AcquireSemaphore");
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		VK_CHECK_RESULT(vkCreateFence(device->getVkDevice(), &fenceSignaledInfo, getVkAllocator(), &presentFence[i]));
		setDebugName(device->getVkDevice(), presentFence[i], "presentFence");
		VK_CHECK_RESULT(vkCreateSemaphore(device->getVkDevice(), &semaphoreInfo, getVkAllocator(), &presentSemaphore[i]));
		setDebugName(device->getVkDevice(), presentSemaphore[i], "PresentSemaphore%u", i);
	}
	// Command buffers
	for (QueueType queue : EnumRange<QueueType>())
	{
		// Create frame command pool.
		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = device->getVkQueueIndex(queue);
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_CHECK_RESULT(vkCreateCommandPool(device->getVkDevice(), &createInfo, getVkAllocator(), &commandPool[EnumToIndex(queue)]));

		// Allocate primary command buffers from frame command pool
		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.commandPool = commandPool[EnumToIndex(queue)];
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VkCommandBuffer cmd;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(device->getVkDevice(), &allocateInfo, &cmd));
		mainCommandEncoders[EnumToIndex(queue)] = mem::akaNew<VulkanCommandEncoder>(
			AllocatorMemoryType::Raw, AllocatorCategory::Graphic,
			device, cmd, queue, false);
	}
}

void VulkanFrame::destroy(VulkanGraphicDevice* device)
{
	vkDestroySemaphore(device->getVkDevice(), acquireSemaphore, getVkAllocator());
	acquireSemaphore = VK_NULL_HANDLE;
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		mem::akaDelete<VulkanCommandEncoder>(mainCommandEncoders[i]);
		vkDestroyCommandPool(device->getVkDevice(), commandPool[i], getVkAllocator());
		commandPool[i] = VK_NULL_HANDLE;
		vkDestroyFence(device->getVkDevice(), presentFence[i], getVkAllocator());
		presentFence[i] = VK_NULL_HANDLE;
		vkDestroySemaphore(device->getVkDevice(), presentSemaphore[i], getVkAllocator());
		presentSemaphore[i] = VK_NULL_HANDLE;
	}
}

void VulkanFrame::wait(VkDevice device)
{
	// Wait for each queue
	VK_CHECK_RESULT(vkWaitForFences(
		device,
		EnumCount<QueueType>(),
		this->presentFence,
		VK_TRUE,
		(std::numeric_limits<uint64_t>::max)()
	));
}

VulkanCommandEncoder* VulkanFrame::allocateCommand(VulkanGraphicDevice* device, QueueType queue)
{
	// Allocate primary command buffers from frame command pool
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.commandPool = commandPool[EnumToIndex(queue)];
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer cmd;
	VK_CHECK_RESULT(vkAllocateCommandBuffers(device->getVkDevice(), &allocateInfo, &cmd));
	// TODO: this invalidate previous pointers...
	commandEncoders[EnumToIndex(queue)].append(mem::akaNew<VulkanCommandEncoder>(AllocatorMemoryType::Raw, AllocatorCategory::Graphic, device, cmd, queue, false));

	return commandEncoders[EnumToIndex(queue)].last();
}

void VulkanFrame::releaseCommand(VulkanCommandEncoder* commandList)
{
	VkCommandBuffer cmd = commandList->getVkCommandBuffer();
	vkFreeCommandBuffers(commandList->getDevice()->getVkDevice(), commandPool[EnumToIndex(commandList->getQueueType())], 1, &cmd);
	commandEncoders[EnumToIndex(commandList->getQueueType())].remove(&commandList);
}

};
};