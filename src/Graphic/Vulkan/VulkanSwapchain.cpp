#include "VulkanSwapchain.h"

#include "VulkanContext.h"
#include "VulkanGraphicDevice.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "../../Platform/GLFW3/PlatformGLFW3.h"

namespace aka {
namespace gfx {

VkSurfaceFormatKHR getSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	uint32_t formatCount;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr));
	std::vector<VkSurfaceFormatKHR> formats;
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

bool isPresentModeAvailable(const std::vector<VkPresentModeKHR>& presentModes, VkPresentModeKHR presentMode)
{
	return presentModes.end() != std::find(presentModes.begin(), presentModes.end(), presentMode);
}

VkPresentModeKHR getPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	uint32_t presentModeCount;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));

	std::vector<VkPresentModeKHR> presentModes;
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
VkExtent2D getSurfaceExtent(VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int w, h;
		glfwGetFramebufferSize(window, &w, &h);
		return VkExtent2D{
			max(capabilities.minImageExtent.width, min(capabilities.maxImageExtent.width, static_cast<uint32_t>(w))),
			max(capabilities.minImageExtent.height, min(capabilities.maxImageExtent.height, static_cast<uint32_t>(h)))
		};;
	}
}

VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
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
	return findSupportedFormat(
		physicalDevice,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VulkanSwapchain::VulkanSwapchain() : 
	m_needRecreation(false),
	m_width(0),
	m_height(0),
	m_surfaceFormat(VK_FORMAT_UNDEFINED),
	m_platform(nullptr),
	m_swapchain(VK_NULL_HANDLE),
	m_imageCount(0),
	m_currentFrameIndex{ 0 },
	m_frames{},
	m_backbuffers{},
	m_colorFormat(TextureFormat::Unknown),
	m_depthFormat(TextureFormat::Unknown),
	m_backbufferTextures{}
{
}

bool VulkanSwapchain::initialize(VulkanGraphicDevice* device, PlatformDevice* platform)
{
	AKA_ASSERT(m_swapchain == VK_NULL_HANDLE, "Cannot call initialize with already initialized swapchain.");
	createSwapchain(device, platform, VK_NULL_HANDLE);
	createImageViews(device);
	createFrames(device);
	return true;
}

void VulkanSwapchain::shutdown(VulkanGraphicDevice* _device)
{
	m_needRecreation = false;
	m_imageCount = 0;
	m_currentFrameIndex = FrameIndex(0);
	m_colorFormat = TextureFormat::Unknown;
	m_depthFormat = TextureFormat::Unknown;
	destroyFrames(_device);
	destroyFramebuffers(_device);
	destroyImageViews(_device);
	destroySwapchain(_device);
}

void VulkanSwapchain::onReceive(const BackbufferResizeEvent& e)
{
	if (e.width != m_width || e.height != m_height)
	{
		m_needRecreation = true;
	}
}

void VulkanSwapchain::recreate(VulkanGraphicDevice* _device)
{
	PlatformGLFW3* glfw3 = reinterpret_cast<PlatformGLFW3*>(m_platform);
	int width = 0, height = 0;
	glfwGetFramebufferSize(glfw3->getGLFW3Handle(), &width, &height);
	while (width == 0 || height == 0) // Wait while window minimized
	{
		glfwGetFramebufferSize(glfw3->getGLFW3Handle(), &width, &height);
		glfwWaitEvents();
	}

	_device->wait();

	destroyImageViews(_device);
	VkSwapchainKHR oldSwapchain = m_swapchain;
	createSwapchain(_device, m_platform, m_swapchain);
	createImageViews(_device);
	vkDestroySwapchainKHR(_device->getVkDevice(), oldSwapchain, nullptr);
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
		recreate(device);
		return nullptr; // Do not draw this frame.
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire swapchain image");
	}
	AKA_ASSERT(imageIndex < getImageCount(), "Invalid image index");
	AKA_ASSERT(MaxFrameInFlight <= getImageCount(), "More frames in flight than image available. May induce bugs in application.");

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
		recreate(device);
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

BackbufferHandle VulkanSwapchain::createBackbuffer(VulkanGraphicDevice* device, RenderPassHandle handle)
{
	RenderPassState state = device->get(handle)->state;
	BackbufferHandle bbhandle = BackbufferHandle{ (void*)std::hash<RenderPassState>{}(state) };
	auto it = m_backbuffers.find(bbhandle);
	if (it != m_backbuffers.end())
		return bbhandle;
	// Create a framebuffer for backbuffer compatible with given render pass.
	Backbuffer backbuffer(String::format("Backbuffer%ul", bbhandle).cstr());
	backbuffer.renderPass = handle;
	for (uint32_t i = 0; i < m_imageCount; i++)
	{
		Attachment color = Attachment{ m_backbufferTextures[i].color, AttachmentFlag::None, 0, 0 };
		Attachment depth = Attachment{ m_backbufferTextures[i].depth, AttachmentFlag::None, 0, 0 };
		FramebufferHandle fb = device->createFramebuffer("Backbuffer", handle, &color, 1, &depth);
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

void VulkanSwapchain::createSwapchain(VulkanGraphicDevice* _device, PlatformDevice* _platform, VkSwapchainKHR _oldSwapchain)
{
	PlatformGLFW3* glfw3 = reinterpret_cast<PlatformGLFW3*>(_platform);
	m_platform = _platform;

	VkSurfaceCapabilitiesKHR capabilities;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device->getVkPhysicalDevice(), _device->getVkSurface(), &capabilities));

	VkSurfaceFormatKHR surfaceFormat = getSurfaceFormat(_device->getVkPhysicalDevice(), _device->getVkSurface());
	VkPresentModeKHR bestMode = getPresentMode(_device->getVkPhysicalDevice(), _device->getVkSurface());
	VkExtent2D extent = getSurfaceExtent(capabilities, glfw3->getGLFW3Handle());
	m_imageCount = capabilities.minImageCount + 1; // Request more than the minimum to avoid driver overhead
	if (capabilities.maxImageCount == 0) // Unlimited count allowed
		capabilities.maxImageCount = 4;
	m_imageCount = clamp(gfx::MaxFrameInFlight + 1, capabilities.minImageCount, capabilities.maxImageCount);
	m_surfaceFormat = surfaceFormat.format;
	m_width = extent.width;
	m_height = extent.height;

	// Create swapchain
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = _device->getVkSurface();
	createInfo.minImageCount = m_imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (true)
		createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; // For blitting operations.

	std::set<uint32_t> singleImageFamilies;
	for (QueueType queue : EnumRange<QueueType>())
		singleImageFamilies.insert(_device->getVkQueueIndex(queue));
	singleImageFamilies.insert(_device->getVkPresentQueueIndex());

	std::vector<uint32_t> singleImageFamiliesData;
	for (uint32_t f : singleImageFamilies)
		singleImageFamiliesData.push_back(f);

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

	VK_CHECK_RESULT(vkCreateSwapchainKHR(_device->getVkDevice(), &createInfo, nullptr, &m_swapchain));
}
void VulkanSwapchain::createImageViews(VulkanGraphicDevice* _device)
{
	VkFormat vk_depthFormat = findDepthFormat(_device->getVkPhysicalDevice());
	VkFormat vk_colorFormat = m_surfaceFormat;
	// TODO convert vulkan format to aka format
	m_colorFormat = TextureFormat::BGRA8;
	m_depthFormat = TextureFormat::Depth32F;
	AKA_ASSERT(VulkanContext::tovk(m_colorFormat) == vk_colorFormat, "Invalid color format");
	AKA_ASSERT(VulkanContext::tovk(m_depthFormat) == vk_depthFormat, "Invalid depth format");

	const bool hasDepth = true;
	const bool hasStencil = false;
	// Get images & layout
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(_device->getVkDevice(), m_swapchain, &m_imageCount, nullptr));
	Vector<VkImage> vk_images(m_imageCount);
	m_backbufferTextures.resize(m_imageCount);
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(_device->getVkDevice(), m_swapchain, &m_imageCount, vk_images.data()));

	for (size_t i = 0; i < m_imageCount; i++)
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
		VK_CHECK_RESULT(vkCreateImageView(_device->getVkDevice(), &viewInfo, nullptr, &view));

		// Create color texture
		String str = String::format("SwapChain%u", i);
		VulkanTexture* vk_colorTexture = _device->m_texturePool.acquire(str.cstr(), m_width, m_height, 1,
			TextureType::Texture2D,
			1, 1,
			m_colorFormat,
			TextureUsage::RenderTarget
		);
		// Cannot transition color images yet as they are not acquired.
		TextureHandle colorTexture = TextureHandle{ vk_colorTexture };
		vk_colorTexture->vk_image = vk_images[i];
		vk_colorTexture->vk_view[0] = view; // Set main image view
		setDebugName(_device->getVkDevice(), vk_images[i], "SwapchainColor", i);
		setDebugName(_device->getVkDevice(), view, "SwapchainColorView", i);
		{
			VkCommandBuffer cmd = VulkanCommandList::createSingleTime("TransitionBackbuffer", _device->getVkDevice(), _device->getVkCommandPool(QueueType::Graphic));
			VulkanTexture::transitionImageLayout(cmd, vk_images[i], ResourceAccessType::Undefined, ResourceAccessType::Present, m_colorFormat);
			VulkanCommandList::endSingleTime(_device->getVkDevice(), _device->getVkCommandPool(QueueType::Graphic), cmd, _device->getVkQueue(QueueType::Graphic));
		}
		// No memory

		// Create depth texture
		gfx::TextureHandle depthTexture;
		if (hasDepth)
		{
			String str = String::format("SwapchainDepthImage%u", i);
			// Layout should be set to default.
			depthTexture = _device->createTexture(
				str.cstr(),
				m_width, m_height, 1,
				TextureType::Texture2D,
				1,
				1,
				m_depthFormat,
				TextureUsage::RenderTarget,
				nullptr
			);
			VulkanTexture* vk_depth = _device->getVk<VulkanTexture>(depthTexture);

			{
				VkCommandBuffer cmd = VulkanCommandList::createSingleTime("TransitionBackbuffer", _device->getVkDevice(), _device->getVkCommandPool(QueueType::Graphic));
				VulkanTexture::transitionImageLayout(cmd, vk_depth->vk_image, ResourceAccessType::Undefined, ResourceAccessType::Present, m_depthFormat);
				VulkanCommandList::endSingleTime(_device->getVkDevice(), _device->getVkCommandPool(QueueType::Graphic), cmd, _device->getVkQueue(QueueType::Graphic));
			}
		}

		m_backbufferTextures[i].color = colorTexture;
		m_backbufferTextures[i].depth = depthTexture;
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
	vkDestroySwapchainKHR(_device->getVkDevice(), m_swapchain, nullptr);
	m_swapchain = VK_NULL_HANDLE;
}
void VulkanSwapchain::destroyImageViews(VulkanGraphicDevice* _device)
{
	for (BackBufferTextures backbuffer : m_backbufferTextures)
	{
		_device->destroy(backbuffer.color);
		_device->destroy(backbuffer.depth);
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
		// Create a framebuffer for backbuffer compatible with given render pass.
		for (FramebufferHandle handle : backbuffer.handles)
		{
			_device->destroy(handle);
		}
		backbuffer.handles.clear();
		for (uint32_t i = 0; i < m_imageCount; i++)
		{
			Attachment color = Attachment{ m_backbufferTextures[i].color, AttachmentFlag::None, 0, 0 };
			Attachment depth = Attachment{ m_backbufferTextures[i].depth, AttachmentFlag::None, 0, 0 };
			FramebufferHandle fb = _device->createFramebuffer("Backbuffer", backbuffer.renderPass, &color, 1, &depth);
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
	ptrdiff_t dist = std::distance(m_frames, &frame);
	AKA_ASSERT(dist < gfx::MaxFrameInFlight, "Invalid index");
	return FrameIndex(static_cast<FrameIndex::Type>(dist));
}

VulkanFrame::VulkanFrame() :
	Frame("VulkanFrame")
{
}

VulkanFrame::VulkanFrame(const char* name) :
	Frame(name)
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
	VK_CHECK_RESULT(vkCreateSemaphore(device->getVkDevice(), &semaphoreInfo, nullptr, &acquireSemaphore));
	setDebugName(device->getVkDevice(), acquireSemaphore, "AcquireSemaphore");
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		VK_CHECK_RESULT(vkCreateFence(device->getVkDevice(), &fenceSignaledInfo, nullptr, &presentFence[i]));
		setDebugName(device->getVkDevice(), presentFence[i], "presentFence");
		VK_CHECK_RESULT(vkCreateSemaphore(device->getVkDevice(), &semaphoreInfo, nullptr, &presentSemaphore[i]));
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

		VK_CHECK_RESULT(vkCreateCommandPool(device->getVkDevice(), &createInfo, nullptr, &commandPool[EnumToIndex(queue)]));

		// Allocate primary command buffers from frame command pool
		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.commandPool = commandPool[EnumToIndex(queue)];
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VkCommandBuffer cmd;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(device->getVkDevice(), &allocateInfo, &cmd));
		mainCommandLists[EnumToIndex(queue)] = VulkanCommandList(device, cmd, queue, false);
	}
}

void VulkanFrame::destroy(VulkanGraphicDevice* device)
{
	vkDestroySemaphore(device->getVkDevice(), acquireSemaphore, nullptr);
	acquireSemaphore = VK_NULL_HANDLE;
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		vkDestroyCommandPool(device->getVkDevice(), commandPool[i], nullptr);
		commandPool[i] = VK_NULL_HANDLE;
		vkDestroyFence(device->getVkDevice(), presentFence[i], nullptr);
		presentFence[i] = VK_NULL_HANDLE;
		vkDestroySemaphore(device->getVkDevice(), presentSemaphore[i], nullptr);
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

VulkanCommandList* VulkanFrame::allocateCommand(VulkanGraphicDevice* device, QueueType queue)
{
	// Allocate primary command buffers from frame command pool
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.commandPool = commandPool[EnumToIndex(queue)];
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer cmd;
	VK_CHECK_RESULT(vkAllocateCommandBuffers(device->getVkDevice(), &allocateInfo, &cmd));
	commandLists[EnumToIndex(queue)].append(VulkanCommandList(device, cmd, queue, false));

	return &commandLists[EnumToIndex(queue)].last();
}

void VulkanFrame::releaseCommand(VulkanCommandList* commandList)
{
	VkCommandBuffer cmd = commandList->getVkCommandBuffer();
	vkFreeCommandBuffers(commandList->getDevice()->getVkDevice(), commandPool[EnumToIndex(commandList->getQueueType())], 1, &cmd);
	commandLists[EnumToIndex(commandList->getQueueType())].remove(commandList);
}

};
};