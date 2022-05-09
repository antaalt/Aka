#pragma once

#include <stdint.h>

#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Graphic/Resource.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/OS/Image.h>

namespace aka {
namespace gfx {

enum class IndexFormat : uint8_t
{
	Unknown,
	UnsignedByte, // D3D do not support it
	UnsignedShort,
	UnsignedInt
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
	UnsignedInt
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
};

struct VertexAttribute
{
	VertexSemantic semantic; // Semantic of the attribute
	VertexFormat format; // Format of the attribute
	VertexType type; // Type of the attribute
};

struct VertexBindingState
{
	static constexpr uint32_t MaxAttributes = 8;

	VertexAttribute attributes[MaxAttributes]; // Attributes
	uint32_t offsets[MaxAttributes]; // Offsets of the attributes in a buffer
	uint32_t count; // Number of attributes
	//uint32_t stride; // Stride of the attributes

	uint32_t stride() const;

	static uint32_t size(VertexFormat format);
	static uint32_t size(VertexType type);
	static uint32_t size(IndexFormat format);
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
	OneMinusSrc1Alpha
};

enum class BlendOp : uint8_t
{
	Unknown,
	Add,
	Subtract,
	ReverseSubtract,
	Min,
	Max
};

enum class BlendMask : uint8_t
{
	None  = (0),
	Red   = (1 << 0),
	Green = (1 << 1),
	Blue  = (1 << 2),
	Alpha = (1 << 3),
	Rgb = Red | Green | Blue,
	Rgba = Red | Green | Blue | Alpha
};

BlendMask operator&(BlendMask lhs, BlendMask rhs);
BlendMask operator|(BlendMask lhs, BlendMask rhs);

struct BlendState
{
	BlendMode colorModeSrc;
	BlendMode colorModeDst;
	BlendOp colorOp;

	BlendMode alphaModeSrc;
	BlendMode alphaModeDst;
	BlendOp alphaOp;

	BlendMask mask;

	uint32_t blendColor;

	bool isEnabled() const;
};

enum class CullMode : uint8_t
{
	None,
	FrontFace,
	BackFace,
	AllFace
};

enum class CullOrder : uint8_t
{
	Unknown,
	ClockWise,
	CounterClockWise,
};

struct CullState
{
	CullMode mode;
	CullOrder order;
};

enum class DepthOp
{
	None,
	Always,
	Never,
	Less,
	Equal,
	LessOrEqual,
	Greater,
	NotEqual,
	GreaterOrEqual
};

struct DepthState
{
	DepthOp compare;
	bool mask;

	bool isEnabled() const;
};

enum class StencilOp
{
	None,
	Never,
	Less,
	LessOrEqual,
	Greater,
	GreaterOrEqual,
	Equal,
	NotEqual,
	Always,
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
};

struct ViewportState
{
	Rect viewport;
	Rect scissor;
};

struct GraphicPipeline;
using GraphicPipelineHandle = ResourceHandle<GraphicPipeline>;

struct GraphicPipeline : Resource
{
	ProgramHandle program;

	PrimitiveType primitive;
	VertexBindingState vertices;
	ShaderBindingState sets[ShaderMaxSetCount];
	FramebufferState framebuffer;
	CullState cull;
	FillState fill;
	DepthState depth;
	StencilState stencil;
	BlendState blend;
	ViewportState viewport;
};

struct ComputePipeline;
using ComputePipelineHandle = ResourceHandle<ComputePipeline>;

struct ComputePipeline : Resource
{
	ProgramHandle program;
	ShaderBindingState sets[ShaderMaxSetCount];
};

struct RaytracingPipeline : Resource
{
	// ...
};

};
};