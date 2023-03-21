#pragma once

#include <Aka/Graphic/Pipeline.h>

#if defined(AKA_USE_VULKAN)

#include "VulkanContext.h"
#include "Vulkanprogram.h"

namespace aka {
namespace gfx {

struct VulkanGraphicPipeline : GraphicPipeline
{
	VkPipeline vk_pipeline;
	VkPipelineLayout vk_pipelineLayout;

	static VkPipeline createVkGraphicPipeline(
		VkDevice device,
		VkRenderPass renderpass,
		VkPipelineLayout pipelineLayout,
		const VulkanShader** shaders,
		uint32_t shaderCount,
		PrimitiveType primitive,
		const VertexAttributeState& vertices,
		const RenderPassState& renderPass,
		const DepthState& depth,
		const StencilState& stencil,
		const ViewportState& viewport,
		const CullState& cull,
		const BlendState& blend,
		const FillState& fill
	);

	static VkVertexInputBindingDescription getVertexBindings(const VertexAttributeState& vertices, VkVertexInputAttributeDescription* attributes, uint32_t count);
};

struct VulkanComputePipeline : ComputePipeline
{
	VkPipeline vk_pipeline;
	VkPipelineLayout vk_pipelineLayout;

	static VkPipeline createVkComputePipeline(
		VkDevice device,
		VkPipelineLayout pipelineLayout,
		const VulkanShader* shader	
	);
};

};
};

#endif