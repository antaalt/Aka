#pragma once

#include "VulkanCommon.hpp"

#include <Aka/Graphic/RenderPass.h>

namespace aka {
namespace gfx {

class VulkanGraphicDevice;

struct VulkanRenderPass : RenderPass
{
	VulkanRenderPass(const char* name, const RenderPassState& state);

	VkRenderPass vk_renderpass; // cached in context. Do not destroy

	void create(VulkanGraphicDevice* device);
	void destroy(VulkanGraphicDevice* device);

	static VkRenderPass createVkRenderPass(VkDevice device, const RenderPassState& state);
};

};
};