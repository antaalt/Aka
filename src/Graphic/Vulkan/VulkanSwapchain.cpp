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
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats;
	if (formatCount == 0)
	{
		Logger::error("No valid format for surface");
		return VkSurfaceFormatKHR{ VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}
	formats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
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

VkPresentModeKHR getPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
	std::vector<VkPresentModeKHR> presentModes;
	if (presentModeCount == 0)
	{
		Logger::error("No valid present mode for surface");
		return bestMode;
	}
	presentModes.resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
	for (const VkPresentModeKHR& availablePresentMode : presentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
		{
			bestMode = availablePresentMode;
			break;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			bestMode = availablePresentMode;
			break;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			bestMode = availablePresentMode;
			break;
		}
	}
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

void VulkanSwapchain::initialize(VulkanGraphicDevice* device, PlatformDevice* platform)
{
	PlatformGLFW3* glfw3 = reinterpret_cast<PlatformGLFW3*>(platform);
	m_platform = platform;

	VkSurfaceCapabilitiesKHR capabilities;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->getVkPhysicalDevice(), device->getVkSurface(), &capabilities));

	VkSurfaceFormatKHR surfaceFormat = getSurfaceFormat(device->getVkPhysicalDevice(), device->getVkSurface());
	VkPresentModeKHR bestMode = getPresentMode(device->getVkPhysicalDevice(), device->getVkSurface());
	VkExtent2D extent = getSurfaceExtent(capabilities, glfw3->getGLFW3Handle());
	m_imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && m_imageCount > capabilities.maxImageCount)
		m_imageCount = capabilities.maxImageCount;

	// Create swapchain
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = device->getVkSurface();
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
		singleImageFamilies.insert(device->getVkQueueIndex(queue));
	singleImageFamilies.insert(device->getVkPresentQueueIndex());

	std::vector<uint32_t> singleImageFamiliesData;
	for (uint32_t f : singleImageFamilies)
		singleImageFamiliesData.push_back(f);

	if (singleImageFamilies.size() > 1)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = (uint32_t)singleImageFamilies.size();
		createInfo.pQueueFamilyIndices = singleImageFamiliesData.data();
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}
	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = bestMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK_RESULT(vkCreateSwapchainKHR(device->getVkDevice(), &createInfo, nullptr, &m_swapchain));

	VkFormat vk_depthFormat = findDepthFormat(device->getVkPhysicalDevice());
	VkFormat vk_colorFormat = surfaceFormat.format;
	// TODO convert vulkan format to aka format
	m_colorFormat = TextureFormat::BGRA8;
	m_depthFormat = TextureFormat::Depth32F;
	AKA_ASSERT(VulkanContext::tovk(m_colorFormat) == vk_colorFormat, "Invalid color format");
	AKA_ASSERT(VulkanContext::tovk(m_depthFormat) == vk_depthFormat, "Invalid depth format");

	const bool hasDepth = true;
	const bool hasStencil = false;
	// Get images & layout
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device->getVkDevice(), m_swapchain, &m_imageCount, nullptr));
	std::vector<VkImage> vk_images(m_imageCount);
	m_backbufferTextures.resize(m_imageCount);
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device->getVkDevice(), m_swapchain, &m_imageCount, vk_images.data()));

	VkCommandBuffer cmd = VulkanCommandList::createSingleTime(device->getVkDevice(), device->getVkCommandPool(QueueType::Graphic));
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
		VK_CHECK_RESULT(vkCreateImageView(device->getVkDevice(), &viewInfo, nullptr, &view));

		// Create color texture
		String str = String::format("SwapChain%u", i);
		VulkanTexture* vk_colorTexture = device->m_texturePool.acquire(str.cstr(), extent.width, extent.height, 1,
			TextureType::Texture2D,
			1, 1,
			m_colorFormat,
			TextureFlag::RenderTarget
		);
		// Cannot transition color images yet as they are not acquired.
		TextureHandle colorTexture = TextureHandle{ vk_colorTexture };
		vk_colorTexture->vk_image = vk_images[i];
		vk_colorTexture->vk_view = view;
		setDebugName(device->getVkDevice(), vk_images[i], "SwapchainColor", i);
		setDebugName(device->getVkDevice(), view, "SwapchainColorView", i);
		// No memory
		
		// Create depth texture
		gfx::TextureHandle depthTexture;
		if (hasDepth)
		{
			String str = String::format("SwapchainDepthImage%u", i);
			// Layout should be set to default.
			depthTexture = device->createTexture(
				str.cstr(),
				extent.width, extent.height, 1,
				TextureType::Texture2D,
				1,
				1,
				m_depthFormat,
				TextureFlag::RenderTarget,
				nullptr
			);
		}
		
		m_backbufferTextures[i].color = colorTexture;
		m_backbufferTextures[i].depth = depthTexture;
	}

	// Frames
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceSignaledInfo = {};
	fenceSignaledInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceSignaledInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (VulkanFrame& frame : m_frames)
	{
		VK_CHECK_RESULT(vkCreateSemaphore(device->getVkDevice(), &semaphoreInfo, nullptr, &frame.acquireSemaphore));
		for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
		{
			VK_CHECK_RESULT(vkCreateSemaphore(device->getVkDevice(), &semaphoreInfo, nullptr, &frame.presentSemaphore[i]));
			VK_CHECK_RESULT(vkCreateFence(device->getVkDevice(), &fenceSignaledInfo, nullptr, &frame.presentFence[i]));

			// Allocate primary command buffers
			VkCommandBufferAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocateInfo.commandBufferCount = 1;
			allocateInfo.commandPool = device->getVkCommandPool(IndexToEnum<QueueType>(i));
			allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

			VkCommandBuffer cmd;
			VK_CHECK_RESULT(vkAllocateCommandBuffers(device->getVkDevice(), &allocateInfo, &cmd));
			m_frames->commandLists[i] = VulkanCommandList(device, cmd, IndexToEnum<QueueType>(i), false);
		}
	}
}

void VulkanSwapchain::shutdown(VulkanGraphicDevice* device)
{
	m_needRecreation = false;
	m_imageCount = 0;
	m_currentFrameIndex.value = 0;
	m_colorFormat = TextureFormat::Unknown;
	m_depthFormat = TextureFormat::Unknown;

	for (auto backbuffer : m_backbuffers)
	{
		backbuffer.first; // TODO: Clear renderpass ref count
		for (FramebufferHandle fb : backbuffer.second.handles)
		{
			device->destroy(fb);
		}
	}
	m_backbuffers.clear();
	for (BackBufferTextures backbuffer : m_backbufferTextures)
	{
		device->destroy(backbuffer.color);
		device->destroy(backbuffer.depth);
	}
	m_backbufferTextures.clear();

	vkDestroySwapchainKHR(device->getVkDevice(), m_swapchain, nullptr);
	m_swapchain = VK_NULL_HANDLE;
	for (VulkanFrame& frame : m_frames)
	{
		vkDestroySemaphore(device->getVkDevice(), frame.acquireSemaphore, nullptr);
		frame.acquireSemaphore = VK_NULL_HANDLE;
		for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
		{
			vkDestroySemaphore(device->getVkDevice(), frame.presentSemaphore[i], nullptr);
			vkDestroyFence(device->getVkDevice(), frame.presentFence[i], nullptr);
			frame.presentFence[i] = VK_NULL_HANDLE;
			frame.presentSemaphore[i] = VK_NULL_HANDLE;
		}
	}
}

void VulkanSwapchain::onReceive(const BackbufferResizeEvent& e)
{
	if (e.width != m_width || e.height != m_height)
	{
		m_needRecreation = true;
	}
}

void VulkanSwapchain::recreate(VulkanGraphicDevice* device)
{
	PlatformGLFW3* glfw3 = reinterpret_cast<PlatformGLFW3*>(m_platform);
	int width = 0, height = 0;
	glfwGetFramebufferSize(glfw3->getGLFW3Handle(), &width, &height);
	while (width == 0 || height == 0) // Wait while window minimized
	{
		glfwGetFramebufferSize(glfw3->getGLFW3Handle(), &width, &height);
		glfwWaitEvents();
	}

	device->wait();
	// Destroy swapchain & recreate it.
	shutdown(device);
	bool changed = (m_width != width) || (m_height != height);
	m_width = width;
	m_height = height;
	initialize(device, m_platform);
	EventDispatcher<BackbufferResizeEvent>::trigger(BackbufferResizeEvent{ m_width, m_height });
}

VulkanFrame* VulkanSwapchain::acquireNextImage(VulkanGraphicDevice* device)
{
	VulkanFrame& vk_frame = m_frames[m_currentFrameIndex.value];

	// Wait for the frame to complete before acquiring it.
	vk_frame.wait(device->getVkDevice());

	uint32_t imageIndex; // Will be initialized by following call
	VkResult result = vkAcquireNextImageKHR(
		device->getVkDevice(),
		m_swapchain,
		(std::numeric_limits<uint64_t>::max)(),
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
	AKA_ASSERT(FrameIndex::MaxInFlight <= getImageCount(), "More frames in flight than image available. May induce bugs in application.");

	// Only reset the fence if we are submitting work
	VkFence fences[EnumCount<QueueType>()] = {
		vk_frame.presentFence[EnumToIndex(QueueType::Graphic)],
		vk_frame.presentFence[EnumToIndex(QueueType::Compute)],
		vk_frame.presentFence[EnumToIndex(QueueType::Copy)],
	};
	vkResetFences(device->getVkDevice(), EnumCount<QueueType>(), fences);

	// Set the index 
	vk_frame.m_image = ImageIndex{ imageIndex };

	// TODO check image finished rendering ?
	// If less image than frames in flight, might be necessary

	return &vk_frame;
}

SwapchainStatus VulkanSwapchain::present(VulkanGraphicDevice* device, VulkanFrame* vk_frame)
{
	VkSwapchainKHR swapChains[] = { m_swapchain };
	uint32_t indices[] = { vk_frame->m_image.value };

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = EnumCount<QueueType>();
	presentInfo.pWaitSemaphores = vk_frame->presentSemaphore;
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
	m_currentFrameIndex.next();
	return status;
}

BackbufferHandle VulkanSwapchain::createBackbuffer(VulkanGraphicDevice* device, RenderPassHandle handle)
{
	RenderPassState state = device->get(handle)->state;
	auto it = m_backbuffers.find(state);
	if (it != m_backbuffers.end())
		return BackbufferHandle{ &it->second };
	// Create a framebuffer for backbuffer compatible with given render pass.
	Backbuffer backbuffer("Backbuffer"); // TODO add hash
	for (uint32_t i = 0; i < m_imageCount; i++)
	{
		Attachment color = Attachment{ m_backbufferTextures[i].color, AttachmentFlag::None, 0, 0 };
		Attachment depth = Attachment{ m_backbufferTextures[i].depth, AttachmentFlag::None, 0, 0 };
		FramebufferHandle fb = device->createFramebuffer("Backbuffer", handle, &color, 1, &depth);
		backbuffer.handles.push_back(fb);
	}
	auto itInsert = m_backbuffers.insert(std::make_pair(device->get(handle)->state, backbuffer));
	AKA_ASSERT(itInsert.second, "Failed to create backbuffer");
	// TODO this might get invalidated when growing the map...
	return BackbufferHandle{ &itInsert.first->second }; // backbuffer handle use render pass handle for now
}

void VulkanSwapchain::destroyBackbuffer(VulkanGraphicDevice* device, BackbufferHandle handle)
{
	/*const Backbuffer* backbuffer = reinterpret_cast<const Backbuffer*>(handle.__data);
	RenderPassState state;
	for (FramebufferHandle handle : backbuffer->handles)
	{
		device->destroy(handle);
		state = device->get(device->get(handle)->renderPass)->state;
	}
	m_backbuffers.erase(state);*/
	// Should ref count state to do not destroy shared backbuffer.
	// For now, do not destroy anything.
}

void VulkanFrame::wait(VkDevice device)
{
	VK_CHECK_RESULT(vkWaitForFences(
		device,
		1,
		&this->presentFence[0], // QueueType::Graphic
		VK_TRUE,
		(std::numeric_limits<uint64_t>::max)()
	));
	VK_CHECK_RESULT(vkWaitForFences(
		device,
		1,
		&this->presentFence[1], // QueueType::Compute
		VK_TRUE,
		(std::numeric_limits<uint64_t>::max)()
	));
	VK_CHECK_RESULT(vkWaitForFences(
		device,
		1,
		&this->presentFence[2], // QueueType::Copy
		VK_TRUE,
		(std::numeric_limits<uint64_t>::max)()
	));
}

};
};