#pragma once

#include "VulkanContext.h"

#include <Aka/Graphic/Framebuffer.h>

namespace aka {
namespace gfx {

class VulkanGraphicDevice;

struct VulkanFramebuffer : Framebuffer
{
	VulkanFramebuffer(const char* name, uint32_t width, uint32_t height, RenderPassHandle handle, const Attachment* colors, uint32_t count, const Attachment* depth);

	VkFramebuffer vk_framebuffer;
	std::vector<VkImageView> vk_views; // specific views to delete with framebuffer

	void create(VulkanGraphicDevice* device);
	void destroy(VulkanGraphicDevice* device);

	static VkFramebuffer createVkFramebuffer(VulkanGraphicDevice* device, VkRenderPass renderpass, const VulkanFramebuffer* framebuffer, std::vector<VkImageView> &views);
};

};
};