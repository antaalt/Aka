#include <Aka/Graphic/RenderPass.h>
#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/OS/Logger.h>

namespace aka {

const Blending Blending::none = {
	BlendMode::One,
	BlendMode::Zero,
	BlendOp::Add,
	BlendMode::One,
	BlendMode::Zero,
	BlendOp::Add,
	BlendMask::Rgba,
	color32(255)
};

const Blending Blending::premultiplied = {
	BlendMode::One,
	BlendMode::OneMinusSrcAlpha,
	BlendOp::Add,
	BlendMode::One,
	BlendMode::OneMinusSrcAlpha,
	BlendOp::Add,
	BlendMask::Rgba,
	color32(255)
};

bool Blending::operator==(const Blending& rhs) const
{
	return colorModeSrc == rhs.colorModeSrc && colorModeDst == rhs.colorModeDst && colorOp == rhs.colorOp && 
		alphaModeSrc == rhs.alphaModeSrc && alphaModeDst == rhs.alphaModeDst && alphaOp == rhs.alphaOp &&
		mask == rhs.mask && blendColor == rhs.blendColor;
}

bool Blending::operator!=(const Blending& rhs) const
{
	return colorModeSrc != rhs.colorModeSrc || colorModeDst != rhs.colorModeDst || colorOp != rhs.colorOp ||
		alphaModeSrc != rhs.alphaModeSrc || alphaModeDst != rhs.alphaModeDst || alphaOp != rhs.alphaOp ||
		mask != rhs.mask || blendColor != rhs.blendColor;
}

bool Blending::enabled() const
{
	return !(colorModeSrc == BlendMode::One && colorModeDst == BlendMode::Zero && alphaModeSrc == BlendMode::One && alphaModeDst == BlendMode::Zero);
}

const Culling Culling::none = {
	CullMode::None,
	CullOrder::CounterClockWise
};

bool Culling::operator==(const Culling& rhs) const
{
	return mode == rhs.mode && order == rhs.order;
}

bool Culling::operator!=(const Culling& rhs) const
{
	return mode != rhs.mode || order != rhs.order;
}

const Depth Depth::none = {
	DepthCompare::None,
	true
};

bool Depth::operator==(const Depth& rhs) const
{
	return compare == rhs.compare && mask == rhs.mask;
}

bool Depth::operator!=(const Depth& rhs) const
{
	return compare != rhs.compare || mask != rhs.mask;
}

const Stencil Stencil::none = {
	Face {
		StencilMode::Keep,
		StencilMode::Keep,
		StencilMode::Keep,
		StencilCompare::None
	},
	Face {
		StencilMode::Keep,
		StencilMode::Keep,
		StencilMode::Keep,
		StencilCompare::None
	},
	0xff,
	0xff
};

const Stencil Stencil::always = {
	Face {
		StencilMode::Replace,
		StencilMode::Replace,
		StencilMode::Replace,
		StencilCompare::Always
	},
	Face {
		StencilMode::Replace,
		StencilMode::Replace,
		StencilMode::Replace,
		StencilCompare::Always
	},
	0xff,
	0xff
};

const Stencil Stencil::equal = {
	Face {
		StencilMode::Keep,
		StencilMode::Keep,
		StencilMode::Keep,
		StencilCompare::Equal
	},
	Face {
		StencilMode::Keep,
		StencilMode::Keep,
		StencilMode::Keep,
		StencilCompare::Equal
	},
	0xff,
	0xff
};

bool Stencil::operator==(const Stencil& rhs) const
{
	return front.stencilFailed == rhs.front.stencilFailed &&
		front.stencilDepthFailed == rhs.front.stencilDepthFailed &&
		front.stencilPassed == rhs.front.stencilPassed &&
		front.mode == rhs.front.mode &&
		back.stencilFailed == rhs.back.stencilFailed &&
		back.stencilDepthFailed == rhs.back.stencilDepthFailed &&
		back.stencilPassed == rhs.back.stencilPassed &&
		back.mode == rhs.back.mode &&
		readMask == rhs.readMask &&
		writeMask == rhs.writeMask;
}

bool Stencil::operator!=(const Stencil& rhs) const
{
	return front.stencilFailed != rhs.front.stencilFailed ||
		front.stencilDepthFailed != rhs.front.stencilDepthFailed ||
		front.stencilPassed != rhs.front.stencilPassed ||
		front.mode != rhs.front.mode ||
		back.stencilFailed != rhs.back.stencilFailed ||
		back.stencilDepthFailed != rhs.back.stencilDepthFailed ||
		back.stencilPassed != rhs.back.stencilPassed ||
		back.mode != rhs.back.mode ||
		readMask != rhs.readMask ||
		writeMask != rhs.writeMask;
}

bool Stencil::enabled() const
{
	return front.mode != StencilCompare::None && back.mode != StencilCompare::None;
}


const Clear Clear::none = {
	ClearMask::None,
	color4f(1.f),
	1.f,
	1
};

void RenderPass::execute()
{
	if (this->material == nullptr)
	{
		Logger::error("No Material set for render pass.");
		return;
	}
	if (this->submesh.mesh == nullptr)
	{
		Logger::warn("No mesh set for render pass.");
		return;
	}
	if (this->submesh.mesh->getVertexAttributeCount() >= this->material->getShader()->getAttributeCount())
	{
		for (uint32_t i = 0; i < this->material->getShader()->getAttributeCount(); i++)
		{
			const VertexAttribute& mesh = this->submesh.mesh->getVertexAttribute(i);
			const VertexAttribute& shader = this->material->getShader()->getAttribute(i);
			if (mesh.semantic != shader.semantic || mesh.format != shader.format || mesh.type != shader.type)
			{
				Logger::warn("Shader and mesh non compatible");
				return;
			}
		}
	}
	else
	{
		Logger::warn("Shader and mesh non compatible");
		return;
	}
	GraphicBackend::render(*this);
}

void ComputePass::execute()
{
	if (this->material == nullptr)
	{
		Logger::error("No Material set for render pass.");
		return;
	}
	GraphicBackend::dispatch(*this);
}

};