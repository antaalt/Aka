#include <Aka/Graphic/Sampler.h>

#include <Aka/Core/Geometry.h>

namespace aka {

uint32_t TextureSampler::mipLevelCount(uint32_t width, uint32_t height)
{
	return static_cast<uint32_t>(floor(log2(max(width, height)))) + 1;
}

const TextureSampler TextureSampler::nearest = {
	TextureFilter::Nearest,
	TextureFilter::Nearest,
	TextureMipMapMode::None,
	TextureWrap::Repeat,
	TextureWrap::Repeat,
	TextureWrap::Repeat,
	1.f
};

const TextureSampler TextureSampler::bilinear = {
	TextureFilter::Linear,
	TextureFilter::Linear,
	TextureMipMapMode::Nearest,
	TextureWrap::Repeat,
	TextureWrap::Repeat,
	TextureWrap::Repeat,
	1.f
};

const TextureSampler TextureSampler::trilinear = {
	TextureFilter::Linear,
	TextureFilter::Linear,
	TextureMipMapMode::Linear,
	TextureWrap::Repeat,
	TextureWrap::Repeat,
	TextureWrap::Repeat,
	1.f
};

bool TextureSampler::operator==(const TextureSampler& rhs) const
{
	return filterMin == rhs.filterMin && filterMag == rhs.filterMag && mipmapMode == rhs.mipmapMode && wrapU == rhs.wrapU && wrapV == rhs.wrapV && wrapW == rhs.wrapW && anisotropy == rhs.anisotropy;
}

bool TextureSampler::operator!=(const TextureSampler& rhs) const
{
	return filterMin != rhs.filterMin || filterMag != rhs.filterMag || mipmapMode != rhs.mipmapMode || wrapU != rhs.wrapU || wrapV != rhs.wrapV || wrapW != rhs.wrapW || anisotropy != rhs.anisotropy;
}


};