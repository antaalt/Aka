#pragma once

#include <stdint.h>

#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Graphic/RenderPass.h>
#include <Aka/Graphic/Resource.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/OS/Image.h>

namespace aka {
namespace gfx {

struct GraphicPipeline;
struct ComputePipeline;

using GraphicPipelineHandle = ResourceHandle<GraphicPipeline>;
using ComputePipelineHandle = ResourceHandle<ComputePipeline>;

static constexpr uint32_t VertexMaxAttributeCount = 8;

enum class IndexFormat : uint8_t
{
	Unknown,

	UnsignedByte, // D3D do not support it
	UnsignedShort,
	UnsignedInt,

	First = UnsignedByte,
	Last = UnsignedInt,
};

enum class VertexFormat : uint8_t
{
	Unknown,

	Half,
	Float,
	Double,
	Byte,
	UnsignedByte,
	Short,
	UnsignedShort,
	Int,
	UnsignedInt,

	First = Half,
	Last = UnsignedInt,
};

enum class VertexType : uint8_t
{
	Unknown,

	Vec2,
	Vec3,
	Vec4,
	Mat2,
	Mat3,
	Mat4,
	Scalar,

	First = Vec2,
	Last = Scalar,
};

enum class VertexSemantic : uint8_t
{
	Unknown,

	Position,
	Normal,
	Tangent,
	TexCoord0,
	TexCoord1,
	TexCoord2,
	TexCoord3,
	Color0,
	Color1,
	Color2,
	Color3,

	First = Position,
	Last = Color3,
};

enum class PrimitiveType : uint8_t
{
	Unknown,

	Points,
	LineStrip,
	LineLoop,
	Lines,
	TriangleStrip,
	TriangleFan,
	Triangles,

	First = Points,
	Last = Triangles,
};

struct VertexAttribute
{
	VertexSemantic semantic; // Semantic of the attribute
	VertexFormat format; // Format of the attribute
	VertexType type; // Type of the attribute
};

struct VertexAttributeState
{
	VertexAttribute attributes[VertexMaxAttributeCount]; // Attributes
	uint32_t offsets[VertexMaxAttributeCount]; // Offsets of the attributes in a buffer
	uint32_t count; // Number of attributes

	uint32_t stride() const;

	static uint32_t size(VertexFormat format);
	static uint32_t size(VertexType type);
	static uint32_t size(IndexFormat format);
	static VertexAttributeState empty() { return VertexAttributeState{}; }

	VertexAttributeState& add(VertexSemantic semantic, VertexFormat format, VertexType type, uint32_t offset = 0)
	{
		AKA_ASSERT(count + 1 < VertexMaxAttributeCount, "Too many vertex attributes");
		offsets[count] = offset; // TODO compute offset here ?
		attributes[count++] = VertexAttribute{ semantic, format, type };
		return *this;
	}
};

struct ClearState
{
	ClearMask mask;
	float color[4];
	float depth;
	uint32_t stencil;
};

enum class FillMode : uint8_t
{
	Fill,
	Line,
	Point
};

struct FillState
{
	FillMode mode;
	float lineWidth;
};

enum class BlendMode : uint8_t
{
	Unknown,

	Zero,
	One,
	SrcColor,
	OneMinusSrcColor,
	DstColor,
	OneMinusDstColor,
	SrcAlpha,
	OneMinusSrcAlpha,
	DstAlpha,
	OneMinusDstAlpha,
	ConstantColor,
	OneMinusConstantColor,
	ConstantAlpha,
	OneMinusConstantAlpha,
	SrcAlphaSaturate,
	Src1Color,
	OneMinusSrc1Color,
	Src1Alpha,
	OneMinusSrc1Alpha,

	First = Zero,
	Last = OneMinusSrc1Alpha,
};

enum class BlendOp : uint8_t
{
	Unknown,

	Add,
	Subtract,
	ReverseSubtract,
	Min,
	Max,

	First = Add,
	Last = Max,
};

enum class ColorMask : uint8_t
{
	None  = 0,
	Red   = 1 << 0,
	Green = 1 << 1,
	Blue  = 1 << 2,
	Alpha = 1 << 3,
	Rgb   = Red | Green | Blue,
	Rgba  = Rgb | Alpha
};
AKA_IMPLEMENT_BITMASK_OPERATOR(ColorMask)

struct BlendState
{
	BlendMode colorModeSrc;
	BlendMode colorModeDst;
	BlendOp colorOp;

	BlendMode alphaModeSrc;
	BlendMode alphaModeDst;
	BlendOp alphaOp;

	ColorMask mask;

	uint32_t blendColor;

	bool isEnabled() const;

	BlendState& setColor(BlendMode src, BlendMode dst, BlendOp op)
	{
		colorModeSrc = src;
		colorModeDst = dst;
		colorOp = op;
		return *this;
	}
	BlendState& setAlpha(BlendMode src, BlendMode dst, BlendOp op)
	{
		alphaModeSrc = src;
		alphaModeDst = dst;
		alphaOp = op;
		return *this;
	}
	BlendState& set(ColorMask mask, uint32_t blendColor)
	{
		this->mask = mask;
		this->blendColor = blendColor;
		return *this;
	}

};

enum class CullMode : uint8_t
{
	Unknown,

	None,
	FrontFace,
	BackFace,
	AllFace,

	First = None,
	Last = AllFace,
};

enum class CullOrder : uint8_t
{
	Unknown,

	ClockWise,
	CounterClockWise,

	First = ClockWise,
	Last = CounterClockWise,
};

struct CullState
{
	CullMode mode;
	CullOrder order;

	CullState& set(CullMode mode, CullOrder order)
	{
		this->mode = mode;
		this->order = order;
		return *this;
	}
};

enum class DepthOp
{
	Unknown,

	None,
	Always,
	Never,
	Less,
	Equal,
	LessOrEqual,
	Greater,
	NotEqual,
	GreaterOrEqual,

	First = None,
	Last = GreaterOrEqual,
};

struct DepthState
{
	DepthOp compare;
	bool mask;

	bool isEnabled() const;

	DepthState& set(DepthOp compare, bool mask = true) 
	{
		this->compare = compare;
		this->mask = mask;
		return *this;
	}

};

enum class StencilOp
{
	Unknown,

	None,
	Never,
	Less,
	LessOrEqual,
	Greater,
	GreaterOrEqual,
	Equal,
	NotEqual,
	Always,

	First = None,
	Last = Always,
};

enum class StencilMode
{
	Unknown,

	Keep,
	Zero,
	Replace,
	Increment,
	IncrementWrap,
	Decrement,
	DecrementWrap,
	Invert,

	First = Keep,
	Last = Invert,
};

struct StencilState
{
	struct Face
	{
		StencilMode stencilFailed;
		StencilMode stencilDepthFailed;
		StencilMode stencilPassed;

		StencilOp compare;
	};
	Face front;
	Face back;

	uint32_t readMask;
	uint32_t writeMask;

	bool isEnabled() const;

	StencilState& setFront(StencilMode fail, StencilMode depthFail, StencilMode pass, StencilOp op) 
	{
		front.stencilFailed = fail;
		front.stencilDepthFailed = depthFail;
		front.stencilPassed = pass;
		front.compare = op;
		return *this;
	}
	StencilState& setBack(StencilMode fail, StencilMode depthFail, StencilMode pass, StencilOp op) 
	{
		back.stencilFailed = fail;
		back.stencilDepthFailed = depthFail;
		back.stencilPassed = pass;
		back.compare = op;
		return *this;
	}
	StencilState& read(uint32_t read) 
	{
		readMask = read;
		return *this;
	}
	StencilState& write(uint32_t write) 
	{
		writeMask = write;
		return *this;
	}
};

struct ViewportState
{
	Rect viewport;
	Rect scissor;

	ViewportState& offset(int32_t x, int32_t y) 
	{
		viewport.x = x;
		viewport.y = y;
		scissor.x = x;
		scissor.y = y;
		return *this;
	}
	ViewportState& size(uint32_t w, uint32_t h)
	{
		viewport.w = w;
		viewport.h = h;
		scissor.w = w;
		scissor.h = h;
		return *this;
	}
};

struct GraphicPipeline : Resource
{
	GraphicPipeline() : Resource("", ResourceType::Pipeline) {}

	ProgramHandle program;

	PrimitiveType primitive;
	VertexAttributeState vertices;
	ShaderBindingState sets[ShaderMaxSetCount];
	RenderPassState renderPass;
	CullState cull;
	FillState fill;
	DepthState depth;
	StencilState stencil;
	BlendState blend;
	ViewportState viewport;
};

struct ComputePipeline : Resource
{
	ComputePipeline() : Resource("", ResourceType::Pipeline) {}

	ProgramHandle program;
	ShaderBindingState sets[ShaderMaxSetCount];
};

struct RaytracingPipeline : Resource
{
	RaytracingPipeline() : Resource("", ResourceType::Pipeline) {}
	// ...
};

// Blending
const BlendState BlendStateNormal = BlendState{ BlendMode::One, BlendMode::Zero, BlendOp::Add, BlendMode::One, BlendMode::Zero, BlendOp::Add, ColorMask::Rgba, 0xffffffff };
const BlendState BlendStateAdditive = BlendState{ BlendMode::One, BlendMode::One, BlendOp::Add, BlendMode::One, BlendMode::One, BlendOp::Add, ColorMask::Rgb, 0xffffffff };
const BlendState BlendStatePremultiplied = BlendState{ BlendMode::One, BlendMode::OneMinusSrcAlpha, BlendOp::Add, BlendMode::One, BlendMode::OneMinusSrcAlpha, BlendOp::Add, ColorMask::Rgba, 0xffffffff };
const BlendState BlendStateDefault = BlendStateNormal;

// Filling
const FillState FillStateFill = FillState{ FillMode::Fill, 1.f };
const FillState FillStateLine = FillState{ FillMode::Line, 1.f };

// Culling
const CullState CullStateDisabled = CullState{ CullMode::None, CullOrder::Unknown };
const CullState CullStateCCW = CullState{ CullMode::BackFace, CullOrder::CounterClockWise };
const CullState CullStateCW = CullState{ CullMode::BackFace, CullOrder::ClockWise };
const CullState CullStateDefault = CullStateDisabled;

// Depth
const DepthState DepthStateDisabled = DepthState{ DepthOp::None, true };
const DepthState DepthStateLess = DepthState{ DepthOp::Less, true };
const DepthState DepthStateLessEqual = DepthState{ DepthOp::LessOrEqual, true };
const DepthState DepthStateDefault = DepthStateDisabled;

// Stencil
const StencilState StencilStateDisabled = StencilState{ StencilState::Face{ StencilMode::Keep, StencilMode::Keep, StencilMode::Keep, StencilOp::None}, StencilState::Face{ StencilMode::Keep, StencilMode::Keep, StencilMode::Keep, StencilOp::None}, 0xffffffff, 0xffffffff };
const StencilState StencilStateDefault = StencilStateDisabled;


bool operator<(const VertexAttributeState& lhs, const VertexAttributeState& rhs);
bool operator>(const VertexAttributeState& lhs, const VertexAttributeState& rhs);
bool operator==(const VertexAttributeState& lhs, const VertexAttributeState& rhs);
bool operator!=(const VertexAttributeState& lhs, const VertexAttributeState& rhs);

};
};

template <>
struct std::hash<aka::gfx::VertexAttributeState>
{
	size_t operator()(const aka::gfx::VertexAttributeState& data) const
	{
		size_t hash = 0;
		aka::hashCombine(hash, data.count);
		for (size_t i = 0; i < data.count; i++)
		{
			aka::hashCombine(hash, static_cast<uint32_t>(data.attributes[i].format));
			aka::hashCombine(hash, static_cast<uint32_t>(data.attributes[i].semantic));
			aka::hashCombine(hash, static_cast<uint32_t>(data.attributes[i].type));
			aka::hashCombine(hash, data.offsets[i]);
		}
		return hash;
	}
};