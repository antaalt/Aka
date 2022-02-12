#pragma once

#include "VulkanContext.h"

#include <Aka/Graphic/Framebuffer.h>

namespace aka {

struct VulkanFramebuffer : Framebuffer
{
	VkRenderPass vk_renderpass; // cached in context. Do not destroy
	VkFramebuffer vk_framebuffer;
	bool isSwapchain;

	static VkRenderPass createVkRenderPass(VkDevice device, const FramebufferState& framebufferDesc, VulkanRenderPassLayout layout);
	static VkFramebuffer createVkFramebuffer(VkDevice device, VkRenderPass renderpass, const Framebuffer* framebuffer);
};

};