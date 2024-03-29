#pragma once

#include <Aka/Graphic/Pipeline.h>

#if defined(AKA_USE_VULKAN)

#include "VulkanContext.h"
#include "Vulkanprogram.h"

namespace aka {
namespace gfx {

struct VulkanGraphicPipeline : GraphicPipeline
{
	VulkanGraphicPipeline(
		const char* name,
		ProgramHandle program,
		PrimitiveType primitive,
		const ShaderPipelineLayout& layout,
		const RenderPassState& renderPass,
		const VertexState& vertices,
		const ViewportState& viewport,
		const DepthState& depth,
		const StencilState& stencil,
		const CullState& culling,
		const BlendState& blending,
		const FillState& fill
	);

	VkPipeline vk_pipeline;
	VkPipelineLayout vk_pipelineLayout;

	void create(VulkanGraphicDevice* device);
	void destroy(VulkanGraphicDevice* device);

	static VkPipeline createVkGraphicPipeline(
		VkDevice device,
		VkRenderPass renderpass,
		VkPipelineLayout pipelineLayout,
		const VulkanShader** shaders,
		uint32_t shaderCount,
		PrimitiveType primitive,
		const VertexState& vertices,
		const RenderPassState& renderPass,
		const DepthState& depth,
		const StencilState& stencil,
		const ViewportState& viewport,
		const CullState& cull,
		const BlendState& blend,
		const FillState& fill
	);
};

struct VulkanComputePipeline : ComputePipeline
{
	VulkanComputePipeline(const char* name, ProgramHandle program, const ShaderPipelineLayout& layout);

	VkPipeline vk_pipeline;
	VkPipelineLayout vk_pipelineLayout;

	void create(VulkanGraphicDevice* device);
	void destroy(VulkanGraphicDevice* device);

	static VkPipeline createVkComputePipeline(
		VkDevice device,
		VkPipelineLayout pipelineLayout,
		const VulkanShader* shader	
	);
};

};
};

#endif