#pragma once
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Graphic/GraphicPool.h>

#if defined(AKA_USE_VULKAN)

#include "VulkanContext.h"


namespace aka {

struct VulkanTexture : Texture
{
	VkDeviceMemory vk_memory;
	VkImage vk_image;
	VkImageView vk_view;
	VkImageLayout vk_layout;

	static VkImage createVkImage(VkDevice device, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags);
	static VkImageView createVkImageView(VkDevice device, VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspect, uint32_t mipLevels, uint32_t layers, uint32_t baseMips = 0, uint32_t baseLayer = 0);
	static VkImageAspectFlags getAspectFlag(TextureFormat format);
	static VkAccessFlags accessFlagForLayout(VkImageLayout layout);

	void generateMips(VkCommandBuffer cmd);
	void transitionImageLayout(
		VkCommandBuffer cmd, 
		VkImageLayout newLayout, 
		VkImageSubresourceRange subResource,
		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
	);
	void insertMemoryBarrier(
		VkCommandBuffer cmd,
		VkImageLayout newLayout,
		VkImageSubresourceRange subResource,
		VkPipelineStageFlags srcStage,
		VkPipelineStageFlags dstStage,
		VkAccessFlags srcAccess,
		VkAccessFlags dstAccess
	);
	void copyBufferToImage(VkCommandBuffer cmd, VkBuffer stagingBuffer);

	void copyFrom(VkCommandBuffer cmd, VulkanTexture* texture);

};

};

#endif