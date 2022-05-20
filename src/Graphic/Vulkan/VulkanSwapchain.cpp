#include "VulkanSwapchain.h"

#include "VulkanContext.h"
#include "VulkanGraphicDevice.h"

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

void VulkanSwapchain::initialize(VulkanGraphicDevice* device, PlatformDevice* platform)
{
	VulkanContext* context = &device->m_context;
	PlatformGLFW3* glfw3 = reinterpret_cast<PlatformGLFW3*>(platform);
	this->platform = platform;

	VkSurfaceCapabilitiesKHR capabilities;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physicalDevice, context->surface, &capabilities));

	VkSurfaceFormatKHR surfaceFormat = getSurfaceFormat(context->physicalDevice, context->surface);
	VkPresentModeKHR bestMode = getPresentMode(context->physicalDevice, context->surface);
	VkExtent2D extent = getSurfaceExtent(capabilities, glfw3->getGLFW3Handle());
	imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		imageCount = capabilities.maxImageCount;

	// Create swapchain
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = context->surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	
	if (true)
		createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; // For blitting operations.

	uint32_t queueFamilyIndices[] = {
		context->graphicQueue.index,
		context->presentQueue.index
	};

	if (context->graphicQueue.index != context->presentQueue.index)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
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

	VK_CHECK_RESULT(vkCreateSwapchainKHR(context->device, &createInfo, nullptr, &swapchain));

	VkFormat vk_depthFormat = findDepthFormat(context->physicalDevice);
	VkFormat vk_colorFormat = surfaceFormat.format;
	// TODO convert vulkan format to aka format
	const TextureFormat colorFormat = TextureFormat::BGRA8;
	const TextureFormat depthFormat = TextureFormat::Depth32F;
	const bool hasDepth = true;
	const bool hasStencil = false;
	// Get images & layout
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(context->device, swapchain, &imageCount, nullptr));
	std::vector<VkImage> vk_images(imageCount);
	backbuffers.resize(imageCount);
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(context->device, swapchain, &imageCount, vk_images.data()));

	FramebufferState fbState{};
	fbState.depth.format = depthFormat;
	fbState.depth.loadOp = AttachmentLoadOp::Load;
	fbState.colors[0].format = colorFormat;
	fbState.colors[0].loadOp = AttachmentLoadOp::Load;
	fbState.count = 1;

	VkCommandBuffer cmd = VulkanCommandList::createSingleTime(context->device, context->commandPool);
	VkRenderPass vk_renderPass = context->getRenderPass(fbState, VulkanRenderPassLayout::Backbuffer);
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
		VK_CHECK_RESULT(vkCreateImageView(context->device, &viewInfo, nullptr, &view));

		// Create color texture
		gfx::TextureHandle colorTexture = TextureHandle{ device->makeTexture(
			extent.width, extent.height, 1,
			1, 1,
			colorFormat,
			TextureType::Texture2D,
			TextureFlag::RenderTarget,
			vk_images[i],
			view,
			VK_NULL_HANDLE, // No memory as its owned by swapchain.
			VK_IMAGE_LAYOUT_UNDEFINED
		) };
		// Transition swapchain color image
		VulkanTexture* vk_colorTexture = get<VulkanTexture>(colorTexture);
		vk_colorTexture->transitionImageLayout(
			cmd,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
		);
		// Create depth texture
		gfx::TextureHandle depthTexture;
		if (hasDepth)
		{
			depthTexture = device->createTexture(
				extent.width, extent.height, 1,
				TextureType::Texture2D,
				1,
				1,
				depthFormat,
				TextureFlag::RenderTarget,
				nullptr
			);
			// Transition swapchain depth image
			VulkanTexture* vk_depthTexture = get<VulkanTexture>(depthTexture);
			AKA_ASSERT(!hasStencil, "Invalid layout");
			vk_depthTexture->transitionImageLayout(
				cmd,
				VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 }
			);
		}
		
		// Create framebuffer
		Attachment color{ colorTexture, AttachmentFlag::None, fbState.colors[0].loadOp, 0, 0 };
		Attachment depth{ depthTexture, AttachmentFlag::None, fbState.depth.loadOp, 0, 0 };
		backbuffers[i] = device->createFramebuffer(&color, 1, hasDepth ? &depth : nullptr);
		get<VulkanFramebuffer>(backbuffers[i])->isSwapchain = true;
	}

	// Frames
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (VulkanFrame& frame : frames)
	{
		VK_CHECK_RESULT(vkCreateSemaphore(context->device, &semaphoreInfo, nullptr, &frame.presentSemaphore));
		VK_CHECK_RESULT(vkCreateSemaphore(context->device, &semaphoreInfo, nullptr, &frame.acquireSemaphore));
		VK_CHECK_RESULT(vkCreateFence(context->device, &fenceInfo, nullptr, &frame.presentFence));
		VK_CHECK_RESULT(vkCreateFence(context->device, &fenceInfo, nullptr, &frame.acquireFence));
	}
}

void VulkanSwapchain::shutdown(VulkanGraphicDevice* device)
{
	VulkanContext* context = &device->m_context;
	for (FramebufferHandle backbuffer : backbuffers)
	{
		// vk_renderpass is cached.
		device->destroy(backbuffer.data->colors[0].texture);
		device->destroy(backbuffer.data->depth.texture);
		device->destroy(backbuffer);
	}

	vkDestroySwapchainKHR(context->device, swapchain, nullptr);
	for (VulkanFrame& frame : frames)
	{
		vkDestroySemaphore(context->device, frame.acquireSemaphore, nullptr);
		vkDestroySemaphore(context->device, frame.presentSemaphore, nullptr);
		vkDestroyFence(context->device, frame.acquireFence, nullptr);
		vkDestroyFence(context->device, frame.presentFence, nullptr);
	}
}

void VulkanSwapchain::onReceive(const BackbufferResizeEvent& e)
{
	needRecreation = true;
}

void VulkanSwapchain::recreate(VulkanGraphicDevice* device)
{
	PlatformGLFW3* glfw3 = reinterpret_cast<PlatformGLFW3*>(platform);
	int width = 0, height = 0;
	glfwGetFramebufferSize(glfw3->getGLFW3Handle(), &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(glfw3->getGLFW3Handle(), &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device->context().device);

	shutdown(device);
	initialize(device, platform);
}

VulkanFrame* VulkanSwapchain::acquireNextImage(VulkanGraphicDevice* device)
{
	VulkanFrame& vk_frame = frames[currentFrameIndex.value];

	// Wait for the frame to complete before acquiring it.
	vk_frame.wait(device->context().device);

	uint32_t imageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(
		device->context().device,
		swapchain,
		(std::numeric_limits<uint64_t>::max)(),
		vk_frame.acquireSemaphore,
		VK_NULL_HANDLE,//vk_frame.acquireFence,
		&imageIndex
	);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreate(device);
		return nullptr; // Do not draw this frame.
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		return nullptr;
	}

	// Only reset the fence if we are submitting work
	vkResetFences(device->context().device, 1, &vk_frame.presentFence);
	
	// Set the index 
	vk_frame.image.value = imageIndex;

	AKA_ASSERT(FrameIndex::MaxInFlight <= imageCount, "more frames in flight than image available. May induce bugs in application.");
	// TODO check image finished rendering ?
	// If less image than frames in flight, might be necessary

	return &vk_frame;
}

void VulkanSwapchain::present(VulkanGraphicDevice* device, VulkanFrame* frame)
{
	VkSemaphore waitSemaphores[] = { frame->presentSemaphore };
	VkSwapchainKHR swapChains[] = { swapchain };

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = waitSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &frame->image.value;

	VkResult result = vkQueuePresentKHR(device->context().presentQueue.queue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || needRecreation)
	{
		needRecreation = false;
		recreate(device);
	}
	else if (result != VK_SUCCESS)
	{
		Logger::error("Failed to present swap chain image!");
	}
	currentFrameIndex.next();
}

void VulkanFrame::wait(VkDevice device)
{
	VK_CHECK_RESULT(vkWaitForFences(
		device,
		1,
		&this->presentFence,
		VK_TRUE,
		(std::numeric_limits<uint64_t>::max)()
	));
}

};
};