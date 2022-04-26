#pragma once

#include <stdint.h>

namespace aka {
namespace gfx {

enum class Filter : uint8_t
{
	Nearest,
	Linear,
};

enum class SamplerAddressMode : uint8_t
{
	Repeat,
	Mirror,
	ClampToEdge,
	ClampToBorder,
};

enum class SamplerMipMapMode : uint8_t
{
	None,
	Nearest,
	Linear,
};

struct Sampler
{
	Filter filterMin;
	Filter filterMag;
	SamplerMipMapMode mipmapMode;
	uint32_t mipLevels;
	SamplerAddressMode wrapU;
	SamplerAddressMode wrapV;
	SamplerAddressMode wrapW;
	float anisotropy;

	static uint32_t mipLevelCount(uint32_t width, uint32_t height);

	static Sampler* create(
		Filter filterMin,
		Filter filterMag,
		SamplerMipMapMode mipmapMode,
		uint32_t mipLevels,
		SamplerAddressMode wrapU,
		SamplerAddressMode wrapV,
		SamplerAddressMode wrapW,
		float anisotropy
	);
	static void destroy(Sampler* sampler);
};

};
};