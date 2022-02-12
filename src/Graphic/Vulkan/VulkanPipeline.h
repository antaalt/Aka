#pragma once

#include <Aka/Graphic/Pipeline.h>

#if defined(AKA_USE_VULKAN)

#include "VulkanContext.h"
#include "Vulkanprogram.h"

namespace aka {

struct VulkanPipeline : Pipeline
{
	VkPipeline vk_pipeline;
	//VkPipelineLayout vk_pipelineLayout; // Cached with descriptor set.

	static VkPipeline VulkanPipeline::createVkPipeline(
		VkDevice device,
		VkRenderPass renderpass,
		VkDescriptorSetLayout* layouts, // TODO get from cache using ShaderBinding as key
		uint32_t layoutCount,
		VkPipelineLayout pipelineLayout,
		VulkanShader** shaders,
		uint32_t shaderCount,
		PrimitiveType primitive,
		const VertexBindingState& vertices,
		const ShaderBindingState& bindings,
		const FramebufferState& framebuffer,
		const DepthState& depth,
		const StencilState& stencil,
		const ViewportState& viewport,
		const CullState& cull,
		const BlendState& blend,
		const FillState& fill
	);

	static VkVertexInputBindingDescription getVertexBindings(const VertexBindingState& vertices, VkVertexInputAttributeDescription* attributes, uint32_t count);
};

};

#endif