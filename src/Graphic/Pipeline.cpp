#include <Aka/Graphic/Pipeline.h>

namespace aka {
namespace gfx {

uint32_t VertexBufferLayout::stride() const
{
	AKA_ASSERT(count < VertexMaxAttributeCount, "Too many bindings");
	uint32_t stride = 0;
	for (uint32_t i = 0; i < count; i++)
		stride += VertexBufferLayout::size(attributes[i].format) * VertexBufferLayout::size(attributes[i].type);
	return stride;
}

uint32_t VertexBufferLayout::size(VertexFormat format)
{
	switch (format)
	{
	case VertexFormat::Half:
		return 2;
	case VertexFormat::Float:
		return 4;
	case VertexFormat::Double:
		return 8;
	case VertexFormat::Byte:
		return 1;
	case VertexFormat::UnsignedByte:
		return 1;
	case VertexFormat::Short:
		return 2;
	case VertexFormat::UnsignedShort:
		return 2;
	case VertexFormat::Int:
		return 4;
	case VertexFormat::UnsignedInt:
		return 4;
	default:
		AKA_NOT_IMPLEMENTED;
		return 0;
	}
}
uint32_t VertexBufferLayout::size(VertexType type)
{
	switch (type)
	{
	case VertexType::Scalar:
		return 1;
	case VertexType::Vec2:
		return 2;
	case VertexType::Vec3:
		return 3;
	case VertexType::Vec4:
		return 4;
	default:
		AKA_NOT_IMPLEMENTED;
		return 0;
	}
}
uint32_t VertexBufferLayout::size(IndexFormat format)
{
	switch (format)
	{
	case IndexFormat::UnsignedByte:
		return 1;
	case IndexFormat::UnsignedShort:
		return 2;
	case IndexFormat::UnsignedInt:
		return 4;
	default:
		AKA_NOT_IMPLEMENTED;
		return 0;
	}
}

bool BlendState::isEnabled() const
{
	return !(
		colorModeSrc == BlendMode::One && colorModeDst == BlendMode::Zero && colorOp == BlendOp::Add &&
		alphaModeSrc == BlendMode::One && alphaModeDst == BlendMode::Zero && alphaOp == BlendOp::Add
	);
}

bool DepthState::isEnabled() const
{
	return compare != DepthOp::None;
}

bool StencilState::isEnabled() const
{
	// TODO mask ?
	return (front.compare != StencilOp::None) || (back.compare != StencilOp::None);
}

GraphicPipeline::GraphicPipeline(
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
	Resource(name, ResourceType::Pipeline),
	program(program),
	primitive(primitive),
	layout(layout),
	vertices(vertices),
	renderPass(renderPass),
	cull(culling),
	fill(fill),
	depth(depth),
	stencil(stencil),
	blend(blending),
	viewport(viewport)
{
}

ComputePipeline::ComputePipeline(const char* name, ProgramHandle program, const ShaderPipelineLayout& layout) :
	Resource(name, ResourceType::Pipeline),
	program(program),
	layout(layout)
{
}

bool operator<(const VertexBufferLayout& lhs, const VertexBufferLayout& rhs)
{
	if (lhs.count < rhs.count) return true;
	else if (lhs.count > rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.attributes[i].semantic < rhs.attributes[i].semantic) return true;
		else if (lhs.attributes[i].semantic > rhs.attributes[i].semantic) return false;
		if (lhs.attributes[i].format < rhs.attributes[i].format) return true;
		else if (lhs.attributes[i].format > rhs.attributes[i].format) return false;
		if (lhs.attributes[i].type < rhs.attributes[i].type) return true;
		else if (lhs.attributes[i].type > rhs.attributes[i].type) return false;
		if (lhs.offsets[i] < rhs.offsets[i]) return true;
		else if (lhs.offsets[i] > rhs.offsets[i]) return false;
	}
	return false; // equal 
}

bool operator>(const VertexBufferLayout& lhs, const VertexBufferLayout& rhs)
{
	if (lhs.count > rhs.count) return true;
	else if (lhs.count < rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.attributes[i].semantic > rhs.attributes[i].semantic) return true;
		else if (lhs.attributes[i].semantic < rhs.attributes[i].semantic) return false;
		if (lhs.attributes[i].format > rhs.attributes[i].format) return true;
		else if (lhs.attributes[i].format < rhs.attributes[i].format) return false;
		if (lhs.attributes[i].type > rhs.attributes[i].type) return true;
		else if (lhs.attributes[i].type < rhs.attributes[i].type) return false;
		if (lhs.offsets[i] > rhs.offsets[i]) return true;
		else if (lhs.offsets[i] < rhs.offsets[i]) return false;
	}
	return false; // equal
}

bool operator==(const VertexBufferLayout& lhs, const VertexBufferLayout& rhs)
{
	if (lhs.count != rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.attributes[i].semantic != rhs.attributes[i].semantic) return false;
		if (lhs.attributes[i].format != rhs.attributes[i].format) return false;
		if (lhs.attributes[i].type != rhs.attributes[i].type) return false;
		if (lhs.offsets[i] != rhs.offsets[i]) return false;
	}
	return true; // equal
}

bool operator!=(const VertexBufferLayout& lhs, const VertexBufferLayout& rhs)
{
	if (lhs.count != rhs.count) return true;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.attributes[i].semantic != rhs.attributes[i].semantic) return true;
		if (lhs.attributes[i].format != rhs.attributes[i].format) return true;
		if (lhs.attributes[i].type != rhs.attributes[i].type) return true;
		if (lhs.offsets[i] != rhs.offsets[i]) return true;
	}
	return false; // equal
}


};
};