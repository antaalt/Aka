#include <Aka/Graphic/Pipeline.h>

namespace aka {
namespace gfx {

uint32_t VertexBindingState::stride() const
{
	if (count > VertexMaxAttributeCount)
		return 0;
	uint32_t stride = 0;
	for (uint32_t i = 0; i < count; i++)
		stride += VertexBindingState::size(attributes[i].format) * VertexBindingState::size(attributes[i].type);
	return stride;
}

uint32_t VertexBindingState::size(VertexFormat format)
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
	}
	return 0;
}
uint32_t VertexBindingState::size(VertexType type)
{
	switch (type)
	{
	case VertexType::Vec2:
		return 2;
	case VertexType::Vec3:
		return 3;
	case VertexType::Vec4:
		return 4;
	case VertexType::Mat2:
		return 4;
	case VertexType::Mat3:
		return 12;
	case VertexType::Mat4:
		return 16;
	case VertexType::Scalar:
		return 1;
	}
	return 0;
}
uint32_t VertexBindingState::size(IndexFormat format)
{
	switch (format)
	{
	case IndexFormat::UnsignedByte:
		return 1;
	case IndexFormat::UnsignedShort:
		return 2;
	case IndexFormat::UnsignedInt:
		return 4;
	}
	return 0;
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

bool operator<(const VertexBindingState& lhs, const VertexBindingState& rhs)
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

bool operator>(const VertexBindingState& lhs, const VertexBindingState& rhs)
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

bool operator==(const VertexBindingState& lhs, const VertexBindingState& rhs)
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

bool operator!=(const VertexBindingState& lhs, const VertexBindingState& rhs)
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