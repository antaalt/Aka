#include <Aka/Graphic/Sampler.h>

#include <Aka/Core/Geometry.h>
#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

Sampler::Sampler(const char* name, Filter min, Filter mag, SamplerMipMapMode mipmapMode, SamplerAddressMode wrapU, SamplerAddressMode wrapV, SamplerAddressMode wrapW, float anisotropy) :
	Resource(name, ResourceType::Sampler),
	filterMin(min),
	filterMag(mag),
	mipmapMode(mipmapMode),
	wrapU(wrapU),
	wrapV(wrapV),
	wrapW(wrapW),
	anisotropy(anisotropy)
{
}

uint32_t Sampler::mipLevelCount(uint32_t width, uint32_t height)
{
	return static_cast<uint32_t>(aka::floor(aka::log2(aka::max(width, height)))) + 1U;
}

};
};