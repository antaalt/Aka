#pragma once
#include <Aka/Graphic/GraphicDevice.h>

#if defined(AKA_USE_VULKAN)

#include "VulkanContext.h"


namespace aka {
namespace gfx {

struct VulkanTexture : Texture
{
	//VulkanTexture() : Texture("", 0, 0, 0, TextureType::Unknown, 0, 0,TextureFormat::Unknown, TextureFlag::None) {}
	VulkanTexture(const char* name, uint32_t width, uint32_t height, uint32_t depth, TextureType type, uint32_t levels, uint32_t layers, TextureFormat format, TextureFlag flags);

	VkDeviceMemory vk_memory;
	VkImage vk_image;
	VkImageView vk_view; // TODO vector to support multi view ?
	VkImageLayout vk_layout; // TODO should not be tied to texture, but to command buffer somehow ?

	void create(VulkanContext& context);
	void destroy(VulkanContext& context);
	void upload(VulkanContext& context, const void* const* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

	static VkImage createVkImage(VkDevice device, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags);
	static VkImageView createVkImageView(VkDevice device, VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspect, uint32_t mipLevels, uint32_t layers, uint32_t baseMips = 0, uint32_t baseLayer = 0);
	static VkImageAspectFlags getAspectFlag(TextureFormat format);
	static VkAccessFlags accessFlagForLayout(VkImageLayout layout);
	static void transitionImageLayout(
		VkCommandBuffer cmd,
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImageSubresourceRange subResource,
		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
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
	void blitFrom(VkCommandBuffer cmd, VulkanTexture* texture, const BlitRegion& src, const BlitRegion& dst, Filter filter);

};

};
};

#endif