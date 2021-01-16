#pragma once

#include "Framebuffer.h"
#include "Mesh.h"
#include "Shader.h"

namespace aka {

enum class BlendMode {
	None,
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

enum class CullMode {
	None,
	FrontFace,
	BackFace,
	AllFace
};

struct RenderPass
{
	// Framebuffer to render to
	Framebuffer::Ptr framebuffer;
	// Mesh to render
	Mesh::Ptr mesh;
	// Offset index in mesh
	size_t indexOffset;
	// Count of index in mesh
	size_t indexCount;
	// Texture to use (use material instead to handle other parameters)
	Texture::Ptr texture;
	// Shader for mesh
	Shader *shader;
	// Blend mode for alpha operation
	BlendMode blend;
	// Culling mode for triangle face
	CullMode cull;
	// Viewport for rendering
	Rect viewport;

	// Execute the render pass
	void execute();
};

}