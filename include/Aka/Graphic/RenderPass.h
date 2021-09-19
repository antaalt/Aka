#pragma once

#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Graphic/Mesh.h>
#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/Material.h>

namespace aka {

enum class BlendMode 
{
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

enum class BlendOp
{
	Add,
	Substract,
	ReverseSubstract,
	Min,
	Max
};

enum class BlendMask
{
	None  = 0,
	Red   = 1,
	Green = 2,
	Blue  = 4,
	Alpha = 8,
	Rgb   = Red | Green | Blue,
	Rgba  = Red | Green | Blue | Alpha 
};

struct Blending
{
	BlendMode colorModeSrc;
	BlendMode colorModeDst;
	BlendOp colorOp;

	BlendMode alphaModeSrc;
	BlendMode alphaModeDst;
	BlendOp alphaOp;

	BlendMask mask;

	color32 blendColor;

	static const Blending none;
	static const Blending premultiplied;

	bool operator==(const Blending& rhs) const;
	bool operator!=(const Blending& rhs) const;

	bool enabled() const;
};

enum class CullMode
{
	None,
	FrontFace,
	BackFace,
	AllFace
};

enum class CullOrder 
{
	ClockWise,
	CounterClockWise,
};

struct Culling 
{
	CullMode mode;
	CullOrder order;

	static const Culling none;

	bool operator==(const Culling& rhs) const;
	bool operator!=(const Culling& rhs) const;
};

enum class DepthCompare 
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

struct Depth 
{
	DepthCompare compare;
	bool mask;

	static const Depth none;

	bool operator==(const Depth& rhs) const;
	bool operator!=(const Depth& rhs) const;
};

enum class StencilCompare 
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
	Keep,
	Zero,
	Replace,
	Increment,
	IncrementWrap,
	Decrement,
	DecrementWrap,
	Invert,
};

struct Stencil
{
	struct Face
	{
		StencilMode stencilFailed;
		StencilMode stencilDepthFailed;
		StencilMode stencilPassed;

		StencilCompare mode;
	};
	Face front;
	Face back;

	uint32_t readMask;
	uint32_t writeMask;

	static const Stencil none;
	static const Stencil always;
	static const Stencil equal;

	bool operator==(const Stencil& rhs) const;
	bool operator!=(const Stencil& rhs) const;

	bool enabled() const;
};

struct Clear
{
	ClearMask mask;
	color4f color;
	float depth;
	int stencil;

	static const Clear none;
};

struct RenderPass
{
	// Framebuffer to render to
	Framebuffer::Ptr framebuffer;
	// Mesh to render
	SubMesh submesh;
	// Material for mesh
	Material::Ptr material;
	// Clear values for framebuffer
	Clear clear;
	// Blending for alpha operation
	Blending blend;
	// Culling for triangle face
	Culling cull;
	// Depth test
	Depth depth;
	// Stencil test
	Stencil stencil;
	// Viewport for rendering
	Rect viewport;
	// Scissor for rendering
	Rect scissor;

	// Execute the render pass
	void execute();
};

struct ComputePass
{
	// Material for mesh
	Material::Ptr material;
	// Number of group for dispatch
	vec3u groupCount;
	// Group size in shader
	vec3u groupSize;

	// Execute the compute pass
	void execute();
};

}