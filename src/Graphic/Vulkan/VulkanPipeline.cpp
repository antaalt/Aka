#include "VulkanPipeline.h"

#include "VulkanGraphicDevice.h"

#if defined(AKA_USE_VULKAN)

namespace aka {
namespace gfx {

VkCullModeFlags tovk(CullMode mode)
{
	switch (mode)
	{
	default:
	case CullMode::None:
		return VK_CULL_MODE_NONE;
	case CullMode::FrontFace:
		return VK_CULL_MODE_FRONT_BIT;
	case CullMode::BackFace:
		return VK_CULL_MODE_BACK_BIT;
	case CullMode::AllFace:
		return VK_CULL_MODE_FRONT_AND_BACK;
	}
}
VkPolygonMode tovk(FillMode mode)
{
	switch (mode)
	{
	default:
	case FillMode::Fill:
		return VK_POLYGON_MODE_FILL;
	case FillMode::Line:
		return VK_POLYGON_MODE_LINE;
	case FillMode::Point:
		return VK_POLYGON_MODE_POINT;
	}
}
VkFrontFace tovk(CullOrder order)
{
	switch (order)
	{
	case CullOrder::ClockWise:
		return VK_FRONT_FACE_CLOCKWISE;
	default:
	case CullOrder::CounterClockWise:
		return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}
}
VkCompareOp tovk(DepthOp compare)
{
	switch (compare)
	{
	case DepthOp::Always:
		return VK_COMPARE_OP_ALWAYS;
	case DepthOp::Never:
		return VK_COMPARE_OP_NEVER;
	default:
	case DepthOp::Less:
		return VK_COMPARE_OP_LESS;
	case DepthOp::Equal:
		return VK_COMPARE_OP_EQUAL;
	case DepthOp::LessOrEqual:
		return VK_COMPARE_OP_LESS_OR_EQUAL;
	case DepthOp::Greater:
		return VK_COMPARE_OP_GREATER;
	case DepthOp::NotEqual:
		return VK_COMPARE_OP_NOT_EQUAL;
	case DepthOp::GreaterOrEqual:
		return VK_COMPARE_OP_GREATER_OR_EQUAL;
	}
}
VkBlendFactor tovk(BlendMode mode)
{
	switch (mode)
	{
	default:
	case BlendMode::Zero:
		return VK_BLEND_FACTOR_ZERO;
	case BlendMode::One:
		return VK_BLEND_FACTOR_ONE;
	case BlendMode::SrcColor:
		return VK_BLEND_FACTOR_SRC_COLOR;
	case BlendMode::OneMinusSrcColor:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
	case BlendMode::DstColor:
		return VK_BLEND_FACTOR_DST_COLOR;
	case BlendMode::OneMinusDstColor:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	case BlendMode::SrcAlpha:
		return VK_BLEND_FACTOR_SRC_ALPHA;
	case BlendMode::OneMinusSrcAlpha:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case BlendMode::DstAlpha:
		return VK_BLEND_FACTOR_DST_ALPHA;
	case BlendMode::OneMinusDstAlpha:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	case BlendMode::ConstantColor:
		return VK_BLEND_FACTOR_CONSTANT_COLOR;
	case BlendMode::OneMinusConstantColor:
		return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
	case BlendMode::ConstantAlpha:
		return VK_BLEND_FACTOR_CONSTANT_ALPHA;
	case BlendMode::OneMinusConstantAlpha:
		return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
	case BlendMode::SrcAlphaSaturate:
		return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
	case BlendMode::Src1Color:
		return VK_BLEND_FACTOR_SRC1_COLOR;
	case BlendMode::OneMinusSrc1Color:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
	case BlendMode::Src1Alpha:
		return VK_BLEND_FACTOR_SRC1_ALPHA;
	case BlendMode::OneMinusSrc1Alpha:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
	}
}
VkColorComponentFlags tovk(BlendMask mask)
{
	VkColorComponentFlags flags = 0;
	if ((mask & BlendMask::Red) == BlendMask::Red)
		flags |= VK_COLOR_COMPONENT_R_BIT;
	if ((mask & BlendMask::Green) == BlendMask::Green)
		flags |= VK_COLOR_COMPONENT_G_BIT;
	if ((mask & BlendMask::Blue) == BlendMask::Blue)
		flags |= VK_COLOR_COMPONENT_B_BIT;
	if ((mask & BlendMask::Alpha) == BlendMask::Alpha)
		flags |= VK_COLOR_COMPONENT_A_BIT;
	return flags;
}
VkBlendOp tovk(BlendOp compare)
{
	switch (compare)
	{
	default:
	case BlendOp::Add:
		return VK_BLEND_OP_ADD;
	case BlendOp::Subtract:
		return VK_BLEND_OP_SUBTRACT;
	case BlendOp::ReverseSubtract:
		return VK_BLEND_OP_REVERSE_SUBTRACT;
	case BlendOp::Min:
		return VK_BLEND_OP_MIN;
	case BlendOp::Max:
		return VK_BLEND_OP_MAX;
	}
}
VkFormat tovk(VertexType type, VertexFormat format)
{
	switch (format)
	{
	case VertexFormat::Half:
		switch (type)
		{
		case VertexType::Scalar: return VK_FORMAT_R16_SFLOAT;
		case VertexType::Vec2: return VK_FORMAT_R16G16_SFLOAT;
		case VertexType::Vec3: return VK_FORMAT_R16G16B16_SFLOAT;
		case VertexType::Vec4: return VK_FORMAT_R16G16B16A16_SFLOAT;
		default: return VK_FORMAT_UNDEFINED;
		}
		break;
	case VertexFormat::Float:
		switch (type)
		{
		case VertexType::Scalar: return VK_FORMAT_R32_SFLOAT;
		case VertexType::Vec2: return VK_FORMAT_R32G32_SFLOAT;
		case VertexType::Vec3: return VK_FORMAT_R32G32B32_SFLOAT;
		case VertexType::Vec4: return VK_FORMAT_R32G32B32A32_SFLOAT;
		default: return VK_FORMAT_UNDEFINED;
		}
		break;
	case VertexFormat::Double:
		switch (type)
		{
		case VertexType::Scalar: return VK_FORMAT_R64_SFLOAT;
		case VertexType::Vec2: return VK_FORMAT_R64G64_SFLOAT;
		case VertexType::Vec3: return VK_FORMAT_R64G64B64_SFLOAT;
		case VertexType::Vec4: return VK_FORMAT_R64G64B64A64_SFLOAT;
		default: return VK_FORMAT_UNDEFINED;
		}
		break;
	case VertexFormat::Byte:
		switch (type)
		{
		case VertexType::Scalar: return VK_FORMAT_R8_SNORM;
		case VertexType::Vec2: return VK_FORMAT_R8G8_SNORM;
		case VertexType::Vec3: return VK_FORMAT_R8G8B8_SNORM;
		case VertexType::Vec4: return VK_FORMAT_R8G8B8A8_SNORM;
		default: return VK_FORMAT_UNDEFINED;
		}
		break;
	case VertexFormat::UnsignedByte:
		switch (type)
		{
		case VertexType::Scalar: return VK_FORMAT_R8_UNORM;
		case VertexType::Vec2: return VK_FORMAT_R8G8_UNORM;
		case VertexType::Vec3: return VK_FORMAT_R8G8B8_UNORM;
		case VertexType::Vec4: return VK_FORMAT_R8G8B8A8_UNORM;
		default: return VK_FORMAT_UNDEFINED;
		}
		break;
	default: return VK_FORMAT_UNDEFINED;
	}
	return VK_FORMAT_UNDEFINED;
}
VkPrimitiveTopology tovk(PrimitiveType primitive)
{
	switch (primitive)
	{
	case PrimitiveType::Points:
		return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case PrimitiveType::LineStrip:
		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case PrimitiveType::LineLoop:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
	case PrimitiveType::Lines:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case PrimitiveType::TriangleStrip:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	case PrimitiveType::TriangleFan:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
	default:
	case PrimitiveType::Unknown:
	case PrimitiveType::Triangles:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}
}

VkShaderStageFlagBits tovk(ShaderType type)
{
	switch (type)
	{
	default:
	case ShaderType::Vertex:
		return VK_SHADER_STAGE_VERTEX_BIT;
	case ShaderType::Fragment:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case ShaderType::Compute:
		return VK_SHADER_STAGE_COMPUTE_BIT;
	case ShaderType::Geometry:
		return VK_SHADER_STAGE_GEOMETRY_BIT;
	}
}

VkPipeline VulkanGraphicPipeline::createVkGraphicPipeline(
	VkDevice device, 
	VkRenderPass renderpass,
	VkPipelineLayout pipelineLayout,
	const VulkanShader** shaders,
	uint32_t shaderCount,
	PrimitiveType primitive,
	const VertexBindingState& vertices,
	const FramebufferState& framebuffer,
	const DepthState& depth,
	const StencilState& stencil,
	const ViewportState& viewport,
	const CullState& cull,
	const BlendState& blend,
	const FillState& fill
)
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = tovk(primitive);
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport vk_viewport{};
	// Negative height & increased Y to fix Vulkan Y as up. Supported in 1.1 core.
	vk_viewport.x = static_cast<float>(viewport.viewport.x);
	vk_viewport.y = static_cast<float>(viewport.viewport.y + viewport.viewport.h);
	vk_viewport.width = static_cast<float>(viewport.viewport.w);
	vk_viewport.height = -static_cast<float>(viewport.viewport.h);
	vk_viewport.minDepth = 0.f;
	vk_viewport.maxDepth = 1.f;

	VkRect2D vk_scissor{};
	vk_scissor.extent.width = viewport.scissor.w;
	vk_scissor.extent.height = viewport.scissor.h;
	vk_scissor.offset.x = viewport.scissor.x;
	vk_scissor.offset.y = viewport.scissor.y;

	VkPipelineViewportStateCreateInfo viewportInfo {};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.viewportCount = 1;
	viewportInfo.pViewports = &vk_viewport;
	viewportInfo.scissorCount = 1;
	viewportInfo.pScissors = &vk_scissor;

	VkPipelineRasterizationStateCreateInfo rasterizationInfo {};
	rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationInfo.depthClampEnable = VK_FALSE;
	rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationInfo.polygonMode = tovk(fill.mode);
	rasterizationInfo.depthBiasEnable = VK_FALSE;
	rasterizationInfo.cullMode = tovk(cull.mode);
	rasterizationInfo.frontFace = tovk(cull.order);
	rasterizationInfo.lineWidth = fill.lineWidth;

	VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
	multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleInfo.sampleShadingEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
	depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.depthTestEnable = depth.isEnabled() ? VK_TRUE : VK_FALSE;
	depthStencilInfo.depthWriteEnable = depth.mask ? VK_TRUE : VK_FALSE;
	depthStencilInfo.depthCompareOp = tovk(depth.compare);
	//depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	//depthStencilInfo.minDepthBounds = 0.0f;
	//depthStencilInfo.maxDepthBounds = 1.0f;
	depthStencilInfo.stencilTestEnable = stencil.isEnabled() ? VK_TRUE : VK_FALSE;

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(framebuffer.count, VkPipelineColorBlendAttachmentState{});
	for (uint32_t i = 0; i < framebuffer.count; i++)
	{
		colorBlendAttachments[i].colorWriteMask = tovk(blend.mask);
		colorBlendAttachments[i].blendEnable = blend.isEnabled() ? VK_TRUE : VK_FALSE;
		colorBlendAttachments[i].srcColorBlendFactor = tovk(blend.colorModeSrc);
		colorBlendAttachments[i].dstColorBlendFactor = tovk(blend.colorModeDst);
		colorBlendAttachments[i].colorBlendOp = tovk(blend.colorOp);
		colorBlendAttachments[i].srcAlphaBlendFactor = tovk(blend.alphaModeSrc);
		colorBlendAttachments[i].dstAlphaBlendFactor = tovk(blend.alphaModeDst);
		colorBlendAttachments[i].alphaBlendOp = tovk(blend.alphaOp);
	}

	VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
	colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
	colorBlendInfo.pAttachments = colorBlendAttachments.data();

	VkVertexInputBindingDescription vertexInputBinding{};
	vertexInputBinding.binding = 0;
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexInputBinding.stride = vertices.stride();

	std::vector<VkVertexInputAttributeDescription> attributeDescription(vertices.count, VkVertexInputAttributeDescription{});
	for (uint32_t i = 0; i < vertices.count; i++)
	{
		const VertexAttribute& attribute = vertices.attributes[i];
		attributeDescription[i].binding = 0;
		attributeDescription[i].offset = vertices.offsets[i];
		attributeDescription[i].format = tovk(attribute.type, attribute.format);
		AKA_ASSERT(attributeDescription[i].format != VK_FORMAT_UNDEFINED, "Unsupported format");
		attributeDescription[i].location = i;
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexInputBinding;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();


	// Prepare all shader stages
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages(shaderCount, VkPipelineShaderStageCreateInfo{});
	for (uint32_t i = 0; i < shaderCount; i++)
	{
		VkPipelineShaderStageCreateInfo& shaderStage = shaderStages[i];
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = tovk(shaders[i]->type);
		shaderStage.module = shaders[i]->vk_module;
		shaderStage.pName = "main";
	}

	VkGraphicsPipelineCreateInfo gfxPipelineInfo{};
	gfxPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gfxPipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	gfxPipelineInfo.pStages = shaderStages.data();
	gfxPipelineInfo.pVertexInputState = &vertexInputInfo;
	gfxPipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	gfxPipelineInfo.pRasterizationState = &rasterizationInfo;
	gfxPipelineInfo.pMultisampleState = &multisampleInfo;
	gfxPipelineInfo.pViewportState = &viewportInfo;
	gfxPipelineInfo.pDepthStencilState = &depthStencilInfo;
	gfxPipelineInfo.pColorBlendState = &colorBlendInfo;
	gfxPipelineInfo.renderPass = renderpass;
	gfxPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	gfxPipelineInfo.subpass = 0;
	gfxPipelineInfo.layout = pipelineLayout;

	VkPipeline pipeline = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &gfxPipelineInfo, nullptr, &pipeline));
	return pipeline;
}

VkVertexInputBindingDescription VulkanGraphicPipeline::getVertexBindings(const VertexBindingState& verticesDesc, VkVertexInputAttributeDescription* attributes, uint32_t count)
{
	AKA_ASSERT(count == verticesDesc.count, "");
	VkVertexInputBindingDescription verticesBindings{};
	verticesBindings.binding = 0;
	verticesBindings.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	verticesBindings.stride = verticesDesc.stride();

	for (uint32_t i = 0; i < verticesDesc.count; i++)
	{
		const VertexAttribute& attribute = verticesDesc.attributes[i];
		attributes[i].binding = 0;
		attributes[i].offset = verticesDesc.offsets[i];
		attributes[i].format = tovk(attribute.type, attribute.format);
		AKA_ASSERT(attributes[i].format != VK_FORMAT_UNDEFINED, "Unsupported format");
		attributes[i].location = i;
	}
	return verticesBindings;
}

GraphicPipelineHandle VulkanGraphicDevice::createGraphicPipeline(
	ProgramHandle program,
	PrimitiveType primitive,
	const FramebufferState& framebuffer,
	const VertexBindingState& vertices,
	const ViewportState& viewport,
	const DepthState& depth,
	const StencilState& stencil,
	const CullState& culling,
	const BlendState& blending,
	const FillState& fill
)
{
	if (program.data == nullptr)
		return GraphicPipelineHandle::null;

	VulkanGraphicPipeline* pipeline = m_graphicPipelinePool.acquire();
	pipeline->program = program;
	pipeline->primitive = primitive;
	pipeline->viewport = viewport;
	pipeline->depth = depth;
	pipeline->stencil = stencil;
	pipeline->cull = culling;
	pipeline->blend = blending;
	pipeline->fill = fill;

	pipeline->framebuffer = framebuffer;
	pipeline->vertices = vertices;
	memcpy(pipeline->sets, program.data->sets, program.data->setCount * sizeof(ShaderBindingState));

	VulkanProgram* vk_program = get<VulkanProgram>(program);
	VkDescriptorSetLayout layouts[ShaderMaxSetCount];
	for (uint32_t i = 0; i < program.data->setCount; i++)
	{
		VulkanContext::ShaderInputData data = m_context.getDescriptorLayout(program.data->sets[i]);
		layouts[i] = data.layout;
	}
	pipeline->vk_pipelineLayout = m_context.getPipelineLayout(layouts, program.data->setCount);
	// Create Pipeline
	std::vector<const VulkanShader*> vk_shaders;
	uint32_t shaderCount = 0;
	VulkanShader* vertex = get<VulkanShader>(vk_program->vertex);
	if (vertex)
	{
		vk_shaders.push_back(vertex);
	}
	VulkanShader* fragment = get<VulkanShader>(vk_program->fragment);
	if (fragment)
	{
		vk_shaders.push_back(fragment);
		AKA_ASSERT(vk_shaders.size() == 2, "Missing vertex shader");
	}
	VulkanShader* geometry = get<VulkanShader>(vk_program->geometry);
	if (geometry)
	{
		vk_shaders.push_back(geometry);
		AKA_ASSERT(vk_shaders.size() == 3, "Missing vertex or fragment shader");
	}
	pipeline->vk_pipeline = VulkanGraphicPipeline::createVkGraphicPipeline(
		m_context.device,
		m_context.getRenderPass(framebuffer, VulkanRenderPassLayout::Unknown),
		pipeline->vk_pipelineLayout,
		vk_shaders.data(),
		(uint32_t)vk_shaders.size(),
		pipeline->primitive,
		pipeline->vertices,
		pipeline->framebuffer,
		pipeline->depth,
		pipeline->stencil,
		pipeline->viewport,
		pipeline->cull,
		pipeline->blend,
		pipeline->fill
	);
	return GraphicPipelineHandle{ pipeline };
}
ComputePipelineHandle VulkanGraphicDevice::createComputePipeline(ProgramHandle program, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	if (program.data == nullptr)
		return ComputePipelineHandle::null;

	VulkanComputePipeline* pipeline = m_computePipelinePool.acquire();
	pipeline->program = program;

	memcpy(pipeline->sets, program.data->sets, program.data->setCount * sizeof(ShaderBindingState));

	VulkanProgram* vk_program = get<VulkanProgram>(program);
	VkDescriptorSetLayout layouts[ShaderMaxSetCount];
	for (uint32_t i = 0; i < program.data->setCount; i++)
	{
		VulkanContext::ShaderInputData data = m_context.getDescriptorLayout(program.data->sets[i]);
		layouts[i] = data.layout;
	}
	pipeline->vk_pipelineLayout = m_context.getPipelineLayout(layouts, program.data->setCount);
	// Create Pipeline
	pipeline->vk_pipeline = VulkanComputePipeline::createVkComputePipeline(
		m_context.device,
		pipeline->vk_pipelineLayout,
		get<VulkanShader>(vk_program->compute)
	);
	return ComputePipelineHandle{ pipeline };
}
void VulkanGraphicDevice::destroy(GraphicPipelineHandle pipeline)
{
	if (pipeline.data == nullptr) return;

	VulkanGraphicPipeline* vk_pipeline = get<VulkanGraphicPipeline>(pipeline);

	vkDestroyPipeline(m_context.device, vk_pipeline->vk_pipeline, nullptr);
	// TODO unref.
	//vkDestroyPipelineLayout(m_context.device, vk_pipeline->vk_pipelineLayout, nullptr);

	m_graphicPipelinePool.release(vk_pipeline);
}

void VulkanGraphicDevice::destroy(ComputePipelineHandle handle)
{
	if (handle.data == nullptr) return;

	VulkanComputePipeline* vk_pipeline = get<VulkanComputePipeline>(handle);

	vkDestroyPipeline(m_context.device, vk_pipeline->vk_pipeline, nullptr);
	// TODO unref.
	//vkDestroyPipelineLayout(m_context.device, vk_pipeline->vk_pipelineLayout, nullptr);

	m_computePipelinePool.release(vk_pipeline);
}

VkPipeline VulkanComputePipeline::createVkComputePipeline(VkDevice device, VkPipelineLayout pipelineLayout, const VulkanShader* shader)
{
	AKA_ASSERT(shader->type == ShaderType::Compute, "Invalid shader type");

	VkPipelineShaderStageCreateInfo shaderStage{};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = tovk(ShaderType::Compute);
	shaderStage.module = shader->vk_module;
	shaderStage.pName = "main"; // This member has to be 'main', regardless of the actual entry point of the shader

	VkComputePipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.basePipelineIndex = -1;
	createInfo.basePipelineHandle = VK_NULL_HANDLE;
	createInfo.layout = pipelineLayout;
	createInfo.stage = shaderStage;

	VkPipeline pipeline;
	VK_CHECK_RESULT(vkCreateComputePipelines(device, nullptr, 1, &createInfo, nullptr, &pipeline));
	return pipeline;
}

};
};

#endif