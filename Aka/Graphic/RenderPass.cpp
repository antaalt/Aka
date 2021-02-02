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
	if (this->shader == nullptr)
	{
		Logger::error("No shader set for render pass.");
		return;
	}
	if (this->texture == nullptr)
	{
		uint8_t data[4] = { 255, 255, 255, 255 };
		this->texture = Texture::create(1, 1, Texture::Format::Rgba, data, Sampler::Filter::Nearest);
		Logger::warn("No texture set for render pass.");
	}
	if (this->mesh == nullptr)
	{
		this->mesh = Mesh::create();
		Logger::warn("No mesh set for render pass.");
	}
	GraphicBackend::render(*this);
}

};