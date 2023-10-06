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
	m_imageCount = capabilities.minImageCount + 1; // Request more than the minimum to avoid driver overhead
	if (capabilities.maxImageCount == 0) // Unlimited count allowed
		capabilities.maxImageCount = 4;
	m_imageCount = clamp(gfx::MaxFrameInFlight + 1, capabilities.minImageCount, capabilities.maxImageCount);

	m_width = extent.width;
	m_height = extent.height;

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
	createInfo.clipped = VK_TRUE; // Clip pixels hidden by another window
	createInfo.oldSwapchain = VK_NULL_HANDLE; // TODO: should pass old swapchain here

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
			TextureUsage::RenderTarget
		);
		// Cannot transition color images yet as they are not acquired.
		TextureHandle colorTexture = TextureHandle{ vk_colorTexture };
		vk_colorTexture->vk_image = vk_images[i];
		vk_colorTexture->vk_view[0] = view; // Set main image view
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
				TextureUsage::RenderTarget,
				nullptr
			);
		}
		
		m_backbufferTextures[i].color = colorTexture;
		m_backbufferTextures[i].depth = depthTexture;
	}

	for (VulkanFrame& frame : m_frames)
	{
		frame.create(device);
	}
}

void VulkanSwapchain::shutdown(VulkanGraphicDevice* device)
{
	m_needRecreation = false;
	m_imageCount = 0;
	m_currentFrameIndex = FrameIndex(0);
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
		frame.destroy(device);
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
		(std::numeric_limits<uint64_t>::max)(),
		vk_frame.semaphore[0],
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
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &vk_frame.semaphore[VulkanFrame::SemaphoreCount - 1];
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
	for (uint32_t i = 0; i < m_imageCount; i++)
	{
		Attachment color = Attachment{ m_backbufferTextures[i].color, AttachmentFlag::None, 0, 0 };
		Attachment depth = Attachment{ m_backbufferTextures[i].depth, AttachmentFlag::None, 0, 0 };
		FramebufferHandle fb = device->createFramebuffer("Backbuffer", handle, &color, 1, &depth);
		backbuffer.handles.push_back(fb);
	}
	auto itInsert = m_backbuffers.insert(std::make_pair(bbhandle, backbuffer));
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

	// Semaphores
	for (uint32_t i = 0; i < VulkanFrame::SemaphoreCount; i++)
	{
		VK_CHECK_RESULT(vkCreateSemaphore(device->getVkDevice(), &semaphoreInfo, nullptr, &semaphore[i]));
	}
	// Fences
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		VK_CHECK_RESULT(vkCreateFence(device->getVkDevice(), &fenceSignaledInfo, nullptr, &presentFence[i]));
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
	for (uint32_t i = 0; i < VulkanFrame::SemaphoreCount; i++)
	{
		vkDestroySemaphore(device->getVkDevice(), semaphore[i], nullptr);
		semaphore[i] = VK_NULL_HANDLE;
	}
	for (uint32_t i = 0; i < EnumCount<QueueType>(); i++)
	{
		vkDestroyCommandPool(device->getVkDevice(), commandPool[i], nullptr);
		commandPool[i] = VK_NULL_HANDLE;
		vkDestroyFence(device->getVkDevice(), presentFence[i], nullptr);
		presentFence[i] = VK_NULL_HANDLE;
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