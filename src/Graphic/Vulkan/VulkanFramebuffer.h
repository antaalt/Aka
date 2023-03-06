#pragma once

#include "VulkanContext.h"

#include <Aka/Graphic/Framebuffer.h>

namespace aka {
namespace gfx {

class VulkanGraphicDevice;

struct VulkanFramebuffer : Framebuffer
{
	VulkanFramebuffer(const char* name, uint32_t width, uint32_t height, const FramebufferState& state, const Attachment* colors, const Attachment* depth);

	VkRenderPass vk_renderpass; // cached in context. Do not destroy
	VkFramebuffer vk_framebuffer;
	bool isSwapchain;
	std::vector<VkImageView> vk_views; // specific views to delete with framebuffer

	void create(VulkanGraphicDevice* device);
	void destroy(VulkanGraphicDevice* device);

	static VkRenderPass createVkRenderPass(VkDevice device, const FramebufferState& framebufferDesc, VulkanRenderPassLayout layout);
	static VkFramebuffer createVkFramebuffer(VulkanGraphicDevice* device, VkRenderPass renderpass, const Framebuffer* framebuffer, std::vector<VkImageView> &views);
};

};
};