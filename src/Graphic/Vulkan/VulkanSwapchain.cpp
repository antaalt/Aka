#include "VulkanSwapchain.h"

#include "VulkanContext.h"
#include "VulkanGraphicDevice.h"

#include "../../Platform/GLFW3/PlatformGLFW3.h"

namespace aka {

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
	TextureFormat colorFormat = TextureFormat::BGRA8; 
	TextureFormat depthFormat = TextureFormat::Depth32F;
	// Get images & layout
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(context->device, swapchain, &imageCount, nullptr));
	std::vector<VkImage> vk_images(imageCount);
	backbuffers.resize(imageCount);
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(context->device, swapchain, &imageCount, vk_images.data()));
	for (size_t i = 0; i < imageCount; i++)
	{
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

		backbuffers[i] = device->m_framebufferPool.acquire();
		backbuffers[i]->framebuffer.count = 1;
		backbuffers[i]->colors[0].layer = 0;
		backbuffers[i]->colors[0].level = 0;
		backbuffers[i]->colors[0].flag = AttachmentFlag::None;
		backbuffers[i]->colors[0].texture = device->makeTexture(
			extent.width, extent.height, 1,
			1, 1,
			colorFormat, 
			TextureType::Texture2D,
			TextureFlag::RenderTarget,
			vk_images[i],
			view,
			VK_NULL_HANDLE,
			VK_IMAGE_LAYOUT_UNDEFINED
		);
		backbuffers[i]->isSwapchain = true;
		backbuffers[i]->framebuffer.depth.format = depthFormat;
		backbuffers[i]->framebuffer.count = 1;
		backbuffers[i]->framebuffer.colors[0].format = colorFormat;
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

	// Create depth buffer
	VkRenderPass vk_renderPass = context->getRenderPass(backbuffers[0]->framebuffer, VulkanRenderPassLayout::Backbuffer);
	for (size_t i = 0; i < imageCount; i++)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = extent.width;
		imageInfo.extent.height = extent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = vk_depthFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkImage depthImage = VK_NULL_HANDLE;
		VK_CHECK_RESULT(vkCreateImage(context->device, &imageInfo, nullptr, &depthImage));

		VkMemoryRequirements memRequirements{};
		vkGetImageMemoryRequirements(context->device, depthImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = context->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
		VK_CHECK_RESULT(vkAllocateMemory(context->device, &allocInfo, nullptr, &depthImageMemory));

		VK_CHECK_RESULT(vkBindImageMemory(context->device, depthImage, depthImageMemory, 0));

		VkImageViewCreateInfo depthViewInfo{};
		depthViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		depthViewInfo.image = depthImage;
		depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthViewInfo.format = vk_depthFormat;
		depthViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthViewInfo.subresourceRange.baseMipLevel = 0;
		depthViewInfo.subresourceRange.levelCount = 1;
		depthViewInfo.subresourceRange.baseArrayLayer = 0;
		depthViewInfo.subresourceRange.layerCount = 1;

		VkImageView depthImageView = VK_NULL_HANDLE;
		VK_CHECK_RESULT(vkCreateImageView(context->device, &depthViewInfo, nullptr, &depthImageView));

		backbuffers[i]->width = extent.width;
		backbuffers[i]->height = extent.height;
		backbuffers[i]->depth.layer = 0;
		backbuffers[i]->depth.level = 0;
		backbuffers[i]->depth.flag = AttachmentFlag::None;
		backbuffers[i]->depth.texture = device->makeTexture(
			extent.width, extent.height, 1,
			1, 1,
			depthFormat,
			TextureType::Texture2D,
			TextureFlag::RenderTarget,
			depthImage,
			depthImageView,
			depthImageMemory,
			VK_IMAGE_LAYOUT_UNDEFINED
		);
		backbuffers[i]->vk_renderpass = vk_renderPass;
		backbuffers[i]->vk_framebuffer = VulkanFramebuffer::createVkFramebuffer(context->device, vk_renderPass, backbuffers[i]);
	}
}

void VulkanSwapchain::shutdown(VulkanGraphicDevice* device)
{
	VulkanContext* context = &device->m_context;
	for (VulkanFramebuffer* backbuffer : backbuffers)
	{
		// vk_renderpass is cached.
		vkDestroyFramebuffer(context->device, reinterpret_cast<VulkanFramebuffer*>(backbuffer)->vk_framebuffer, nullptr);
		vkDestroyImageView(context->device, reinterpret_cast<VulkanTexture*>(backbuffer->colors[0].texture)->vk_view, nullptr);
		device->destroy(backbuffer->depth.texture);
		device->m_framebufferPool.release(backbuffer);
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

VulkanFrame* VulkanSwapchain::acquireNextImage(VulkanContext* context)
{
	VulkanFrame& vk_frame = frames[currentFrameIndex.value];

	// Wait for the frame to complete before acquiring it.
	vk_frame.wait(context->device);

	uint32_t imageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(
		context->device,
		swapchain,
		(std::numeric_limits<uint64_t>::max)(),
		vk_frame.acquireSemaphore,
		VK_NULL_HANDLE,//vk_frame.acquireFence,
		&imageIndex
	);
	if (result != VK_SUCCESS)
	{
		Logger::error("Failed to acquire next swapchain image.");
		return nullptr;
	}

	// Set the index 
	vk_frame.needRecreation = (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR);
	vk_frame.image.value = imageIndex;

	AKA_ASSERT(FrameIndex::MaxInFlight <= imageCount, "more frames in flight than image available. May induce bugs in application.");
	// TODO check image finished rendering ?
	// If less image than frames in flight, might be necessary

	return &vk_frame;
}

void VulkanSwapchain::present(VulkanContext* context, VulkanFrame* frame)
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

	VkResult result = vkQueuePresentKHR(context->presentQueue.queue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		VK_CHECK_RESULT(vkQueueWaitIdle(context->presentQueue.queue));
		//return true;
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}
	currentFrameIndex.next();
	//return false;
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