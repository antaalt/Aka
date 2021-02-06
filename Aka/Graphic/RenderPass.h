#pragma once

#include "Framebuffer.h"
#include "Mesh.h"
#include "Shader.h"
#include "ShaderMaterial.h"

namespace aka {

enum class BlendMode {
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

	static Blending none();
	static Blending normal();

	bool operator==(const Blending& rhs) const;
	bool operator!=(const Blending& rhs) const;

	bool enabled() const;
};

enum class CullMode {
	None,
	FrontFace,
	BackFace,
	AllFace
};

enum class CullOrder {
	ClockWise,
	CounterClockWise,
};

struct Culling {
	CullMode mode;
	CullOrder order;

	bool operator==(const Culling& rhs) const;
	bool operator!=(const Culling& rhs) const;
};

enum class DepthCompare {
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

struct RenderPass
{
	// Framebuffer to render to
	Framebuffer::Ptr framebuffer;
	// Mesh to render
	Mesh::Ptr mesh;
	// Offset index in mesh
	uint32_t indexOffset;
	// Count of index in mesh
	uint32_t indexCount;
	// Material for mesh
	ShaderMaterial::Ptr material;
	// Blending for alpha operation
	Blending blend;
	// Culling for triangle face
	Culling cull;
	// Depth mode
	DepthCompare depth;
	// Viewport for rendering
	Rect viewport;

	// Execute the render pass
	void execute();
};

}