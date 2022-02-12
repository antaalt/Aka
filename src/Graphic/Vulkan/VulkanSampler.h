#pragma once
#if defined(AKA_USE_VULKAN)
#include <Aka/Graphic/Sampler.h>

#include "VulkanContext.h"

namespace aka {

struct VulkanSampler : Sampler
{
	VkSampler vk_sampler;

	static VkSampler createVkSampler(
		VkDevice device, VkPhysicalDevice physicalDevice,
		VkFilter vk_filterMin,
		VkFilter vk_filterMag,
		VkSamplerMipmapMode vk_mipmapMode,
		VkSamplerAddressMode vk_wrapU,
		VkSamplerAddressMode vk_wrapV,
		VkSamplerAddressMode vk_wrapW,
		float anisotropy
	);
};

}

#endif