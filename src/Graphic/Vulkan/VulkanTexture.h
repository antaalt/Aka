#pragma once
#include <Aka/Graphic/GraphicDevice.h>

#if defined(AKA_USE_VULKAN)

#include "VulkanContext.h"


namespace aka {
namespace gfx {

class VulkanGraphicDevice;

ResourceAccessType getInitialResourceAccessType(TextureFormat format, TextureUsage flags);

struct VulkanTexture : Texture
{
	VulkanTexture(const char* name, uint32_t width, uint32_t height, uint32_t depth, TextureType type, uint32_t levels, uint32_t layers, TextureFormat format, TextureUsage flags);

	VkDeviceMemory vk_memory;
	VkImage vk_image;
	Vector<VkImageView> vk_view;

	void create(VulkanGraphicDevice* context);
	void destroy(VulkanGraphicDevice* context);

	static VkImage createVkImage(VkDevice device, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags);
	static VkImageView createVkImageView(VkDevice device, VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspect, uint32_t mipCount, uint32_t layerCount, uint32_t baseMips = 0, uint32_t baseLayer = 0);
	static VkImageAspectFlags getAspectFlag(TextureFormat format);

	static void transitionImageLayout(
		VkCommandBuffer cmd,
		VkImage image,
		ResourceAccessType oldLayout,
		ResourceAccessType newLayout,
		TextureFormat format,
		uint32_t level = 0,
		uint32_t levelCount = 1,
		uint32_t layer = 0,
		uint32_t layerCount = 1
	);
	static void insertMemoryBarrier(
		VkCommandBuffer cmd,
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImageSubresourceRange subResource,
		VkPipelineStageFlags srcStage,
		VkPipelineStageFlags dstStage,
		VkAccessFlags srcAccess,
		VkAccessFlags dstAccess
	);

	VkImageView getMainImageView() const;
	VkImageView getImageView(VulkanGraphicDevice* device, uint32_t layer, uint32_t mipLevel);

	void generateMips(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkCommandBuffer cmd, VkBuffer stagingBuffer);

	void copyFrom(VkCommandBuffer cmd, VulkanTexture* texture);
	void blitFrom(VkCommandBuffer cmd, VulkanTexture* texture, const BlitRegion& src, const BlitRegion& dst, Filter filter);

};

};
};

#endif