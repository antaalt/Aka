#include <Aka/Graphic/Pipeline.h>

namespace aka {

uint32_t VertexBindingState::stride() const
{
	if (count > MaxAttributes)
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
	case aka::VertexFormat::Half:
		return 2;
	case aka::VertexFormat::Float:
		return 4;
	case aka::VertexFormat::Double:
		return 8;
	case aka::VertexFormat::Byte:
		return 1;
	case aka::VertexFormat::UnsignedByte:
		return 1;
	case aka::VertexFormat::Short:
		return 2;
	case aka::VertexFormat::UnsignedShort:
		return 2;
	case aka::VertexFormat::Int:
		return 4;
	case aka::VertexFormat::UnsignedInt:
		return 4;
	}
	return 0;
}
uint32_t VertexBindingState::size(VertexType type)
{
	switch (type)
	{
	case aka::VertexType::Vec2:
		return 2;
	case aka::VertexType::Vec3:
		return 3;
	case aka::VertexType::Vec4:
		return 4;
	case aka::VertexType::Mat2:
		return 4;
	case aka::VertexType::Mat3:
		return 12;
	case aka::VertexType::Mat4:
		return 16;
	case aka::VertexType::Scalar:
		return 1;
	}
	return 0;
}
uint32_t VertexBindingState::size(IndexFormat format)
{
	switch (format)
	{
	case aka::IndexFormat::UnsignedByte:
		return 1;
	case aka::IndexFormat::UnsignedShort:
		return 2;
	case aka::IndexFormat::UnsignedInt:
		return 4;
	}
	return 0;
}

BlendMask operator&(BlendMask lhs, BlendMask rhs)
{
	return static_cast<BlendMask>(
		static_cast<std::underlying_type<BlendMask>::type>(lhs) &
		static_cast<std::underlying_type<BlendMask>::type>(rhs)
	);
}

BlendMask operator|(BlendMask lhs, BlendMask rhs)
{
	return static_cast<BlendMask>(
		static_cast<std::underlying_type<BlendMask>::type>(lhs) &
		static_cast<std::underlying_type<BlendMask>::type>(rhs)
	);
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


};