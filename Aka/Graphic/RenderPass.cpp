#include "RenderPass.h"
#include "GraphicBackend.h"
#include "../OS/Logger.h"

namespace aka {

Blending Blending::none()
{
	Blending blending{};
	blending.colorModeSrc = BlendMode::One;
	blending.colorModeDst = BlendMode::Zero;
	blending.colorOp = BlendOp::Add;
	blending.alphaModeSrc = BlendMode::One;
	blending.alphaModeDst = BlendMode::Zero;
	blending.alphaOp = BlendOp::Add;
	blending.mask = BlendMask::Rgba;
	blending.blendColor = color32(255);
	return blending;
}

Blending Blending::normal()
{
	Blending blending{};
	blending.colorModeSrc = BlendMode::One;
	blending.colorModeDst = BlendMode::OneMinusSrcAlpha;
	blending.colorOp = BlendOp::Add;
	blending.alphaModeSrc = BlendMode::One;
	blending.alphaModeDst = BlendMode::OneMinusSrcAlpha;
	blending.alphaOp = BlendOp::Add;
	blending.mask = BlendMask::Rgba;
	blending.blendColor = color32(255);
	return blending;
}

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

void RenderPass::execute()
{
	if (this->material == nullptr)
	{
		Logger::error("No Material set for render pass.");
		return;
	}
	if (this->mesh == nullptr)
	{
		Logger::warn("No mesh set for render pass.");
		return;
	}
	GraphicBackend::render(*this);
}

bool Culling::operator==(const Culling& rhs) const
{
	return mode == rhs.mode && order == rhs.order;
}

bool Culling::operator!=(const Culling& rhs) const
{
	return mode != rhs.mode || order != rhs.order;
}

bool Depth::operator==(const Depth& rhs) const
{
	return compare == rhs.compare && mask == rhs.mask;
}

bool Depth::operator!=(const Depth& rhs) const
{
	return compare != rhs.compare || mask != rhs.mask;
}

Stencil Stencil::none()
{
	Stencil stencil{};
	stencil.front.mode = StencilMode::None;
	stencil.front.stencilFailed = StencilOp::Keep;
	stencil.front.stencilDepthFailed = StencilOp::Keep;
	stencil.front.stencilPassed = StencilOp::Keep;
	stencil.back.mode = StencilMode::None;
	stencil.back.stencilFailed = StencilOp::Keep;
	stencil.back.stencilDepthFailed = StencilOp::Keep;
	stencil.back.stencilPassed = StencilOp::Keep;
	stencil.readMask = 0xff;
	stencil.writeMask = 0xff;
	return stencil;
}

Stencil Stencil::equal()
{
	Stencil stencil{};
	stencil.front.mode = StencilMode::Equal;
	stencil.front.stencilFailed = StencilOp::Keep;
	stencil.front.stencilDepthFailed = StencilOp::Keep;
	stencil.front.stencilPassed = StencilOp::Keep;
	stencil.back.mode = StencilMode::None;
	stencil.back.stencilFailed = StencilOp::Keep;
	stencil.back.stencilDepthFailed = StencilOp::Keep;
	stencil.back.stencilPassed = StencilOp::Keep;
	stencil.readMask = 0xff;
	stencil.writeMask = 0xff;
	return stencil;
}

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
	return front.mode != StencilMode::None && back.mode != StencilMode::None;
}

};