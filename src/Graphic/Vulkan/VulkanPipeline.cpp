#include "VulkanPipeline.h"

#include "VulkanGraphicDevice.h"

#if defined(AKA_USE_VULKAN)

namespace aka {
namespace gfx {

VkCullModeFlags tovk(CullMode mode)
{
	switch (mode)
	{
	case CullMode::None:
		return VK_CULL_MODE_NONE;
	case CullMode::FrontFace:
		return VK_CULL_MODE_FRONT_BIT;
	case CullMode::BackFace:
		return VK_CULL_MODE_BACK_BIT;
	case CullMode::AllFace:
		return VK_CULL_MODE_FRONT_AND_BACK;
	default:
		AKA_ASSERT(false, "invalid enum");
		return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
	}
}
VkPolygonMode tovk(FillMode mode)
{
	switch (mode)
	{
	case FillMode::Fill:
		return VK_POLYGON_MODE_FILL;
	case FillMode::Line:
		return VK_POLYGON_MODE_LINE;
	case FillMode::Point:
		return VK_POLYGON_MODE_POINT;
	default:
		AKA_ASSERT(false, "invalid enum");
		return VK_POLYGON_MODE_MAX_ENUM;
	}
}
VkFrontFace tovk(CullOrder order)
{
	switch (order)
	{
	case CullOrder::ClockWise:
		return VK_FRONT_FACE_CLOCKWISE;
	case CullOrder::CounterClockWise:
		return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	default:
		AKA_ASSERT(false, "invalid enum");
		return VK_FRONT_FACE_MAX_ENUM;
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
	default:
		AKA_ASSERT(false, "invalid enum");
		[[fallthrough]];
	case DepthOp::None: // if disabled
		return VK_COMPARE_OP_NEVER;
	}
}
VkBlendFactor tovk(BlendMode mode)
{
	switch (mode)
	{
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
	default:
		AKA_ASSERT(false, "invalid enum");
		return VK_BLEND_FACTOR_MAX_ENUM;
	}
}
VkColorComponentFlags tovk(ColorMask mask)
{
	VkColorComponentFlags flags = 0;
	if (has(mask, ColorMask::Red))
		flags |= VK_COLOR_COMPONENT_R_BIT;
	if (has(mask, ColorMask::Green))
		flags |= VK_COLOR_COMPONENT_G_BIT;
	if (has(mask, ColorMask::Blue))
		flags |= VK_COLOR_COMPONENT_B_BIT;
	if (has(mask, ColorMask::Alpha))
		flags |= VK_COLOR_COMPONENT_A_BIT;
	return flags;
}
VkBlendOp tovk(BlendOp compare)
{
	switch (compare)
	{
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
	default:
		AKA_ASSERT(false, "invalid enum");
		return VK_BLEND_OP_MAX_ENUM;
	}
}
VkVertexInputRate tovk(VertexStepRate rate)
{
	switch (rate)
	{
	case VertexStepRate::Vertex:
		return VK_VERTEX_INPUT_RATE_VERTEX;
	case VertexStepRate::Instance:
		return VK_VERTEX_INPUT_RATE_INSTANCE;
	default:
		AKA_ASSERT(false, "invalid enum");
		return VK_VERTEX_INPUT_RATE_MAX_ENUM;
	}
	// VK_VERTEX_INPUT_RATE_VERTEX; //VK_VERTEX_INPUT_RATE_INSTANCE if instance
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
		case VertexType::Scalar: return VK_FORMAT_R8_SINT;
		case VertexType::Vec2: return VK_FORMAT_R8G8_SINT;
		case VertexType::Vec3: return VK_FORMAT_R8G8B8_SINT;
		case VertexType::Vec4: return VK_FORMAT_R8G8B8A8_SINT;
		default: return VK_FORMAT_UNDEFINED;
		}
		break;
	case VertexFormat::UnsignedByte:
		switch (type)
		{
		case VertexType::Scalar: return VK_FORMAT_R8_UINT;
		case VertexType::Vec2: return VK_FORMAT_R8G8_UINT;
		case VertexType::Vec3: return VK_FORMAT_R8G8B8_UINT;
		case VertexType::Vec4: return VK_FORMAT_R8G8B8A8_UINT;
		default: return VK_FORMAT_UNDEFINED;
		}
		break;
	case VertexFormat::Int:
		switch (type)
		{
		case VertexType::Scalar: return VK_FORMAT_R32_SINT;
		case VertexType::Vec2: return VK_FORMAT_R32G32_SINT;
		case VertexType::Vec3: return VK_FORMAT_R32G32B32_SINT;
		case VertexType::Vec4: return VK_FORMAT_R32G32B32A32_SINT;
		default: return VK_FORMAT_UNDEFINED;
		}
		break;
	case VertexFormat::UnsignedInt:
		switch (type)
		{
		case VertexType::Scalar: return VK_FORMAT_R32_UINT;
		case VertexType::Vec2: return VK_FORMAT_R32G32_UINT;
		case VertexType::Vec3: return VK_FORMAT_R32G32B32_UINT;
		case VertexType::Vec4: return VK_FORMAT_R32G32B32A32_UINT;
		default: return VK_FORMAT_UNDEFINED;
		}
		break;
	default: 
		AKA_NOT_IMPLEMENTED;
		return VK_FORMAT_UNDEFINED;
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
	case ShaderType::Task:
		return VK_SHADER_STAGE_TASK_BIT_EXT;
	case ShaderType::Mesh:
		return VK_SHADER_STAGE_MESH_BIT_EXT;
	}
}

VulkanGraphicPipeline::VulkanGraphicPipeline(
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
) :
	GraphicPipeline(name, program, primitive, layout, renderPass, vertices, viewport, depth, stencil, culling, blending, fill),
	vk_pipeline(VK_NULL_HANDLE),
	vk_pipelineLayout(VK_NULL_HANDLE)
{
}

void VulkanGraphicPipeline::create(VulkanGraphicDevice* device)
{
	VulkanProgram* vk_program = device->getVk<VulkanProgram>(program);
	
	AKA_ASSERT(vk_program->isCompatible(layout), "Using incompatible binding layout for given shader");

	VkDescriptorSetLayout layouts[ShaderMaxSetCount];
	for (uint32_t i = 0; i < layout.setCount; i++)
	{
		VkDescriptorSetLayout vk_layout = device->getVkDescriptorSetLayout(layout.sets[i]);
		layouts[i] = vk_layout;
	}
	VkPushConstantRange constants[ShaderMaxConstantCount];
	for (uint32_t i = 0; i < layout.constantCount; i++)
	{
		constants[i].offset = layout.constants[i].offset;
		constants[i].size = layout.constants[i].size;
		constants[i].stageFlags = VulkanContext::tovk(layout.constants[i].shader);
	}
	vk_pipelineLayout = device->getVkPipelineLayout(layouts, layout.setCount, constants, layout.constantCount);
	// Create Pipeline
	Vector<const VulkanShader*> vk_shaders;
	uint32_t shaderCount = 0;

	for (ShaderType type : EnumRange<ShaderType>())
	{
		if (VulkanShader* shader = device->getVk<VulkanShader>(vk_program->shaders[EnumToIndex(type)]))
		{
			vk_shaders.append(shader);
		}
	}
	if (asBool(viewport.flags & ViewportFlags::BackbufferAutoResize))
	{
		viewport.viewport.x = 0;
		viewport.viewport.y = 0;
		viewport.scissor.x = 0;
		viewport.scissor.y = 0;
		device->getBackbufferSize(viewport.viewport.w, viewport.viewport.h);
		viewport.scissor.w = viewport.viewport.w;
		viewport.scissor.h = viewport.viewport.h;
	}
	// Here read flag & setup 
	vk_pipeline = VulkanGraphicPipeline::createVkGraphicPipeline(
		device->getVkDevice(),
		device->getVkRenderPass(renderPass),
		vk_pipelineLayout,
		vk_shaders.data(),
		(uint32_t)vk_shaders.size(),
		primitive,
		vertices,
		renderPass,
		depth,
		stencil,
		viewport,
		cull,
		blend,
		fill
	);
}

void VulkanGraphicPipeline::destroy(VulkanGraphicDevice* device)
{
	vkDestroyPipeline(device->getVkDevice(), vk_pipeline, getVkAllocator());
	// TODO unref.
	//vkDestroyPipelineLayout(m_context.device, vk_pipeline->vk_pipelineLayout, nullptr);

	vk_pipelineLayout = VK_NULL_HANDLE;
	vk_pipeline = VK_NULL_HANDLE;

}

VkPipeline VulkanGraphicPipeline::createVkGraphicPipeline(
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
	depthStencilInfo.depthWriteEnable = (depth.isEnabled() && depth.mask) ? VK_TRUE : VK_FALSE;
	depthStencilInfo.depthCompareOp = tovk(depth.compare);
	//depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	//depthStencilInfo.minDepthBounds = 0.0f;
	//depthStencilInfo.maxDepthBounds = 1.0f;
	depthStencilInfo.stencilTestEnable = stencil.isEnabled() ? VK_TRUE : VK_FALSE;

	Vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(renderPass.count, VkPipelineColorBlendAttachmentState{});
	for (uint32_t i = 0; i < renderPass.count; i++)
	{
		colorBlendAttachments[i].colorWriteMask = tovk(blend.mask);
		colorBlendAttachments[i].blendEnable = blend.isEnabled() ? VK_TRUE : VK_FALSE; // is this ok ?
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


	Vector<VkVertexInputBindingDescription> vertexInputBindings(vertices.count, VkVertexInputBindingDescription{});
	Vector<VkVertexInputAttributeDescription> attributeDescriptions;
	uint32_t bindingOffset = 0;
	for (uint32_t iBuffer = 0; iBuffer < vertices.count; iBuffer++)
	{
		const VertexBufferLayout& layout = vertices.bufferLayout[iBuffer];
		vertexInputBindings[iBuffer].binding = iBuffer;
		vertexInputBindings[iBuffer].inputRate = tovk(layout.stepRate);
		vertexInputBindings[iBuffer].stride = vertices.bufferLayout[iBuffer].stride();
		for (uint32_t iBinding = 0; iBinding < layout.count; iBinding++)
		{
			VkVertexInputAttributeDescription& attributeDescription = attributeDescriptions.emplace();
			const VertexAttribute& attribute = layout.attributes[iBinding];
			attributeDescription.binding = iBuffer;
			attributeDescription.offset = vertices.bufferLayout[iBuffer].offsets[iBinding]; // Relative to stride.
			attributeDescription.format = tovk(attribute.type, attribute.format);
			attributeDescription.location = iBinding + bindingOffset; // Shared between buffers
			AKA_ASSERT(attributeDescription.format != VK_FORMAT_UNDEFINED, "Unsupported format");
		}
		bindingOffset += layout.count;
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
	vertexInputInfo.pVertexBindingDescriptions = vertexInputBindings.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


	// Prepare all shader stages
	Vector<VkPipelineShaderStageCreateInfo> shaderStages(shaderCount, VkPipelineShaderStageCreateInfo{});
	for (uint32_t i = 0; i < shaderCount; i++)
	{
		VkPipelineShaderStageCreateInfo& shaderStage = shaderStages[i];
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = tovk(shaders[i]->type);
		shaderStage.module = shaders[i]->vk_module;
		shaderStage.pName = shaders[i]->entryPoint.cstr();
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
	// TODO: cache
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &gfxPipelineInfo, getVkAllocator(), &pipeline));
	return pipeline;
}

GraphicPipelineHandle VulkanGraphicDevice::createGraphicPipeline(
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
)
{
	if (get(program) == nullptr)
		return GraphicPipelineHandle::null;

	VulkanGraphicPipeline* vk_pipeline = m_graphicPipelinePool.acquire(name, program, primitive, layout, renderPass, vertices, viewport, depth, stencil, culling, blending, fill);
	vk_pipeline->create(this);

	return GraphicPipelineHandle{ vk_pipeline };
}

ComputePipelineHandle VulkanGraphicDevice::createComputePipeline(const char* name, ProgramHandle program, const ShaderPipelineLayout& layout)
{
	if (get(program) == nullptr)
		return ComputePipelineHandle::null;

	VulkanComputePipeline* vk_pipeline = m_computePipelinePool.acquire(name, program, layout);
	vk_pipeline->create(this);

	return ComputePipelineHandle{ vk_pipeline };
}

void VulkanGraphicDevice::destroy(GraphicPipelineHandle pipeline)
{
	if (get(pipeline) == nullptr) return;

	VulkanGraphicPipeline* vk_pipeline = getVk<VulkanGraphicPipeline>(pipeline);

	vk_pipeline->destroy(this);

	m_graphicPipelinePool.release(vk_pipeline);
}

void VulkanGraphicDevice::destroy(ComputePipelineHandle handle)
{
	if (get(handle) == nullptr) return;

	VulkanComputePipeline* vk_pipeline = getVk<VulkanComputePipeline>(handle);

	vk_pipeline->destroy(this);

	m_computePipelinePool.release(vk_pipeline);
}

const GraphicPipeline* VulkanGraphicDevice::get(GraphicPipelineHandle handle)
{
	return static_cast<const GraphicPipeline*>(handle.__data);
}

const ComputePipeline* VulkanGraphicDevice::get(ComputePipelineHandle handle)
{
	return static_cast<const ComputePipeline*>(handle.__data);
}

VulkanComputePipeline::VulkanComputePipeline(const char* name, ProgramHandle program, const ShaderPipelineLayout& layout) :
	ComputePipeline(name, program, layout),
	vk_pipeline(VK_NULL_HANDLE),
	vk_pipelineLayout(VK_NULL_HANDLE)
{
}

void VulkanComputePipeline::create(VulkanGraphicDevice* device)
{
	VulkanProgram* vk_program = device->getVk<VulkanProgram>(program);

	AKA_ASSERT(vk_program->isCompatible(layout), "Using incompatible binding layout for given shader");

	VkDescriptorSetLayout layouts[ShaderMaxSetCount]{};
	for (uint32_t i = 0; i < layout.setCount; i++)
	{
		VkDescriptorSetLayout vk_layout = device->getVkDescriptorSetLayout(layout.sets[i]);
		layouts[i] = vk_layout;
	}
	VkPushConstantRange constants[ShaderMaxConstantCount];
	for (uint32_t i = 0; i < layout.constantCount; i++)
	{
		constants[i].offset = layout.constants[i].offset;
		constants[i].size = layout. constants[i].size;
		constants[i].stageFlags = VulkanContext::tovk(layout.constants[i].shader);
	}
	vk_pipelineLayout = device->getVkPipelineLayout(layouts, layout.setCount, constants, layout.constantCount);
	// Create Pipeline
	vk_pipeline = VulkanComputePipeline::createVkComputePipeline(
		device->getVkDevice(),
		vk_pipelineLayout,
		device->getVk<VulkanShader>(vk_program->shaders[EnumToIndex(ShaderType::Compute)])
	);
}

void VulkanComputePipeline::destroy(VulkanGraphicDevice* device)
{
	vkDestroyPipeline(device->getVkDevice(), vk_pipeline, getVkAllocator());
	// TODO unref.
	//vkDestroyPipelineLayout(m_context.device, vk_pipeline->vk_pipelineLayout, nullptr);

	vk_pipelineLayout = VK_NULL_HANDLE;
	vk_pipeline = VK_NULL_HANDLE;
}

VkPipeline VulkanComputePipeline::createVkComputePipeline(VkDevice device, VkPipelineLayout pipelineLayout, const VulkanShader* shader)
{
	AKA_ASSERT(shader->type == ShaderType::Compute, "Invalid shader type");

	VkPipelineShaderStageCreateInfo shaderStage{};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = tovk(ShaderType::Compute);
	shaderStage.module = shader->vk_module;
	shaderStage.pName = shader->entryPoint.cstr();

	VkComputePipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.basePipelineIndex = -1;
	createInfo.basePipelineHandle = VK_NULL_HANDLE;
	createInfo.layout = pipelineLayout;
	createInfo.stage = shaderStage;

	VkPipeline pipeline;
	VK_CHECK_RESULT(vkCreateComputePipelines(device, nullptr, 1, &createInfo, getVkAllocator(), &pipeline));
	return pipeline;
}

};
};

#endif