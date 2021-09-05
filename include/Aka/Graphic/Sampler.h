#pragma once

#include <stdint.h>
#include <memory>

namespace aka {

enum class TextureFilter {
	Nearest,
	Linear,
};

enum class TextureWrap {
	Repeat,
	Mirror,
	ClampToEdge,
	ClampToBorder,
};

enum class TextureMipMapMode {
	None,
	Nearest,
	Linear,
};

struct TextureSampler {
	TextureFilter filterMin;
	TextureFilter filterMag;
	TextureMipMapMode mipmapMode;
	TextureWrap wrapU;
	TextureWrap wrapV;
	TextureWrap wrapW;
	float anisotropy;

	static uint32_t mipLevelCount(uint32_t width, uint32_t height);

	static const TextureSampler nearest;
	static const TextureSampler bilinear;
	static const TextureSampler trilinear;

	bool operator==(const TextureSampler& rhs) const;
	bool operator!=(const TextureSampler& rhs) const;
};

}; // namespace aka