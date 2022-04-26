#include <Aka/Graphic/Sampler.h>

#include <Aka/Core/Geometry.h>
#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

uint32_t Sampler::mipLevelCount(uint32_t width, uint32_t height)
{
	return static_cast<uint32_t>(floor(log2(max(width, height)))) + 1;
}

Sampler* Sampler::create(Filter filterMin, Filter filterMag, SamplerMipMapMode mipmapMode, uint32_t mipLevels, SamplerAddressMode wrapU, SamplerAddressMode wrapV, SamplerAddressMode wrapW, float anisotropy)
{
	return Application::app()->graphic()->createSampler(
		filterMin, filterMag,
		mipmapMode,
		mipLevels,
		wrapU, wrapV, wrapW, 
		anisotropy
	);
}

void Sampler::destroy(Sampler* sampler)
{
	Application::app()->graphic()->destroy(sampler);
}

};
};