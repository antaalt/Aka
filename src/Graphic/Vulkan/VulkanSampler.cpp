#if defined(AKA_USE_VULKAN)

#include "VulkanSampler.h"
#include "VulkanGraphicDevice.h"

namespace aka {
namespace gfx {

VkFilter tovk(Filter filter)
{
	switch (filter)
	{
	case Filter::Nearest:
		return VK_FILTER_NEAREST;
	default:
	case Filter::Linear:
		return VK_FILTER_LINEAR;
	}
}

VkSamplerMipmapMode tovk(SamplerMipMapMode mips)
{
	switch (mips)
	{
	default:
	case SamplerMipMapMode::Nearest:
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case SamplerMipMapMode::Linear:
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
}

VkSamplerAddressMode tovk(SamplerAddressMode mode)
{
	switch (mode)
	{
	default:
	case SamplerAddressMode::Repeat:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case SamplerAddressMode::Mirror:
		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case SamplerAddressMode::ClampToEdge:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case SamplerAddressMode::ClampToBorder:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	}
}

SamplerHandle VulkanGraphicDevice::createSampler(
	const char* name,
	Filter filterMin,
	Filter filterMag,
	SamplerMipMapMode mipmapMode,
	SamplerAddressMode wrapU,
	SamplerAddressMode wrapV,
	SamplerAddressMode wrapW,
	float anisotropy
)
{
	VulkanSampler* vk_sampler = m_samplerPool.acquire(
		name,
		filterMin, filterMag,
		mipmapMode,
		wrapU, wrapV, wrapW,
		anisotropy
	);
	vk_sampler->create(m_context);

	return SamplerHandle{ vk_sampler };
}

void VulkanGraphicDevice::destroy(SamplerHandle sampler)
{
	if (sampler == SamplerHandle::null) return;

	VulkanSampler* vk_sampler = getVk<VulkanSampler>(sampler);
	vk_sampler->destroy(m_context);
	m_samplerPool.release(vk_sampler);
}

const Sampler* VulkanGraphicDevice::get(SamplerHandle handle)
{
	return static_cast<const Sampler*>(handle.__data);
}

VulkanSampler::VulkanSampler(const char* name, Filter min, Filter mag, SamplerMipMapMode mipmapMode, SamplerAddressMode wrapU, SamplerAddressMode wrapV, SamplerAddressMode wrapW, float anisotropy) :
	Sampler(name, min, mag, mipmapMode, wrapU, wrapV, wrapW, anisotropy),
	vk_sampler(VK_NULL_HANDLE)
{
}

void VulkanSampler::create(VulkanContext& context)
{
	VkFilter vk_filterMin = tovk(filterMin);
	VkFilter vk_filterMag = tovk(filterMag);
	VkSamplerMipmapMode vk_mipmapMode = tovk(mipmapMode);
	VkSamplerAddressMode vk_wrapU = tovk(wrapU);
	VkSamplerAddressMode vk_wrapV = tovk(wrapV);
	VkSamplerAddressMode vk_wrapW = tovk(wrapW);

	vk_sampler = VulkanSampler::createVkSampler(
		context.device,
		context.physicalDevice,
		vk_filterMin,
		vk_filterMag,
		vk_mipmapMode,
		10U, // TODO: max mip count ?
		vk_wrapU,
		vk_wrapV,
		vk_wrapW,
		anisotropy
	);
	setDebugName(context.device, vk_sampler, "VkSampler_", name);
}

void VulkanSampler::destroy(VulkanContext& context)
{
	vkDestroySampler(context.device, vk_sampler, nullptr);
	vk_sampler = VK_NULL_HANDLE;
}

VkSampler VulkanSampler::createVkSampler(
	VkDevice device, VkPhysicalDevice physicalDevice,
	VkFilter vk_filterMin,
	VkFilter vk_filterMag,
	VkSamplerMipmapMode vk_mipmapMode,
	uint32_t mipLevels,
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
	samplerInfo.maxLod = static_cast<float>(mipLevels);

	VkSampler sampler = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateSampler(device, &samplerInfo, nullptr, &sampler));

	return sampler;
}
};
};

#endif