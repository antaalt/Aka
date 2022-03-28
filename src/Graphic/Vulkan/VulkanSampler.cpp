#if defined(AKA_USE_VULKAN)

#include "VulkanSampler.h"
#include "VulkanGraphicDevice.h"

namespace aka {

VkFilter tovk(Filter filter)
{
	switch (filter)
	{
	case aka::Filter::Nearest:
		return VK_FILTER_NEAREST;
	default:
	case aka::Filter::Linear:
		return VK_FILTER_LINEAR;
	}
}

VkSamplerMipmapMode tovk(SamplerMipMapMode mips)
{
	switch (mips)
	{
	default:
	case aka::SamplerMipMapMode::Nearest:
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case aka::SamplerMipMapMode::Linear:
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
}

VkSamplerAddressMode tovk(SamplerAddressMode mode)
{
	switch (mode)
	{
	default:
	case aka::SamplerAddressMode::Repeat:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case aka::SamplerAddressMode::Mirror:
		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case aka::SamplerAddressMode::ClampToEdge:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case aka::SamplerAddressMode::ClampToBorder:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	}
}

Sampler* VulkanGraphicDevice::createSampler(
	Filter filterMin,
	Filter filterMag,
	SamplerMipMapMode mipmapMode,
	SamplerAddressMode wrapU,
	SamplerAddressMode wrapV,
	SamplerAddressMode wrapW,
	float anisotropy
)
{
	VulkanSampler* vk_sampler = m_samplerPool.acquire();

	vk_sampler->filterMin = filterMin;
	vk_sampler->filterMag = filterMag;
	vk_sampler->mipmapMode = mipmapMode;
	vk_sampler->wrapU = wrapU;
	vk_sampler->wrapV = wrapV;
	vk_sampler->wrapW = wrapW;
	vk_sampler->anisotropy = anisotropy;

	VkFilter vk_filterMin = tovk(filterMin);
	VkFilter vk_filterMag = tovk(filterMag);
	VkSamplerMipmapMode vk_mipmapMode = tovk(mipmapMode);
	VkSamplerAddressMode vk_wrapU = tovk(wrapU);
	VkSamplerAddressMode vk_wrapV = tovk(wrapV);
	VkSamplerAddressMode vk_wrapW = tovk(wrapW);

	vk_sampler->vk_sampler = VulkanSampler::createVkSampler(
		m_context.device, 
		m_context.physicalDevice,
		vk_filterMin,
		vk_filterMag,
		vk_mipmapMode,
		vk_wrapU,
		vk_wrapV,
		vk_wrapW,
		anisotropy
	);

	return vk_sampler;
}

void VulkanGraphicDevice::destroy(Sampler* sampler)
{
	VulkanSampler* vk_sampler = reinterpret_cast<VulkanSampler*>(sampler);
	vkDestroySampler(m_context.device, vk_sampler->vk_sampler, nullptr);
	m_samplerPool.release(vk_sampler);
}

VkSampler VulkanSampler::createVkSampler(
	VkDevice device, VkPhysicalDevice physicalDevice,
	VkFilter vk_filterMin,
	VkFilter vk_filterMag,
	VkSamplerMipmapMode vk_mipmapMode,
	VkSamplerAddressMode vk_wrapU,
	VkSamplerAddressMode vk_wrapV,
	VkSamplerAddressMode vk_wrapW,
	float anisotropy
)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = vk_filterMag;
	samplerInfo.minFilter = vk_filterMin;
	samplerInfo.addressModeU = vk_wrapU;
	samplerInfo.addressModeV = vk_wrapV;
	samplerInfo.addressModeW = vk_wrapW;
	samplerInfo.anisotropyEnable = VK_TRUE;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	// Used for PCF
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = vk_mipmapMode;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkSampler sampler = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateSampler(device, &samplerInfo, nullptr, &sampler));

	return sampler;
}
}

#endif