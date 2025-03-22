#pragma once
#if defined(AKA_USE_VULKAN)
#include <Aka/Graphic/Sampler.h>

#include "VulkanCommon.hpp"

namespace aka {
namespace gfx {

class VulkanGraphicDevice;

struct VulkanSampler : Sampler
{
	VulkanSampler(const char* name, Filter min, Filter mag, SamplerMipMapMode mipmapMode, SamplerAddressMode wrapU, SamplerAddressMode wrapV, SamplerAddressMode wrapW, float anisotropy);

	VkSampler vk_sampler;

	void create(VulkanGraphicDevice* device);
	void destroy(VulkanGraphicDevice* device);

	static VkSampler createVkSampler(
		VkDevice device, VkPhysicalDevice physicalDevice,
		VkFilter vk_filterMin,
		VkFilter vk_filterMag,
		VkSamplerMipmapMode vk_mipmapMode,
		uint32_t mipLevels,
		VkSamplerAddressMode vk_wrapU,
		VkSamplerAddressMode vk_wrapV,
		VkSamplerAddressMode vk_wrapW,
		float anisotropy
	);
};

};
};

#endif