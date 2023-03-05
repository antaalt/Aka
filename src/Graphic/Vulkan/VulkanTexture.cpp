#if defined(AKA_USE_VULKAN)

#include "VulkanTexture.h"
#include "VulkanGraphicDevice.h"

namespace aka {
namespace gfx {

VkImageAspectFlags VulkanTexture::getAspectFlag(TextureFormat format)
{
	if (Texture::isColor(format))
		return VK_IMAGE_ASPECT_COLOR_BIT;
	else if (Texture::isDepthStencil(format))
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	else if (Texture::isDepth(format))
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	else if (Texture::isStencil(format))
		return VK_IMAGE_ASPECT_STENCIL_BIT;
	else
		return VK_IMAGE_ASPECT_COLOR_BIT;
}
VkAccessFlags VulkanTexture::accessFlagForLayout(VkImageLayout layout)
{
	switch (layout)
	{
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		return VK_ACCESS_HOST_WRITE_BIT;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		return VK_ACCESS_TRANSFER_WRITE_BIT;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return VK_ACCESS_SHADER_READ_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		return VK_ACCESS_TRANSFER_READ_BIT;
	default:
		return 0;
	}
}

void VulkanTexture::transitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subResource, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
{
	VulkanTexture::insertMemoryBarrier(
		cmd, 
		image,
		oldLayout, newLayout,
		subResource,
		srcStage,
		dstStage,
		accessFlagForLayout(oldLayout),
		accessFlagForLayout(newLayout)
	);
}

void VulkanTexture::transitionImageLayout(
	VkCommandBuffer cmd,
	VkImageLayout newLayout, 
	VkImageSubresourceRange subResource,
	VkPipelineStageFlags srcStage,
	VkPipelineStageFlags dstStage
)
{
	insertMemoryBarrier(
		cmd, newLayout,
		subResource,
		srcStage,
		dstStage,
		accessFlagForLayout(vk_layout),
		accessFlagForLayout(newLayout)
	);
}

void VulkanTexture::insertMemoryBarrier(VkCommandBuffer cmd, VkImageLayout newLayout, VkImageSubresourceRange subResource, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkAccessFlags srcAccess, VkAccessFlags dstAccess)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = vk_layout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = vk_image;
	barrier.subresourceRange = subResource;
	barrier.srcAccessMask = srcAccess;
	barrier.dstAccessMask = dstAccess;

	vkCmdPipelineBarrier(
		cmd,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
	vk_layout = newLayout;
}

void VulkanTexture::copyBufferToImage(VkCommandBuffer cmd, VkBuffer stagingBuffer)
{
	VkImageAspectFlags aspectMask = getAspectFlag(format);
	// Copy buffer to image
	{
		AKA_ASSERT((width & (width - 1)) == 0 && levels > 1 || levels == 1, "Width not a power of two");
		AKA_ASSERT((height & (height - 1)) == 0 && levels > 1 || levels == 1, "Height not a power of two");
		for (uint32_t iLayer = 0; iLayer < layers; iLayer++)
		{
			// TODO restore levels
			//for (uint32_t iLevel = 0; iLevel < levels; iLevel++)
			uint32_t iLevel = 0;
			{
				VkBufferImageCopy region{};
				region.bufferOffset = width * height * size(format) * iLayer;
				region.bufferRowLength = 0;
				region.bufferImageHeight = 0;

				region.imageSubresource.aspectMask = aspectMask;
				region.imageSubresource.mipLevel = iLevel;
				region.imageSubresource.baseArrayLayer = iLayer;
				region.imageSubresource.layerCount = 1;

				region.imageOffset = { 0, 0, 0 };
				region.imageExtent = { width >> iLevel, height >> iLevel, 1 };
				vkCmdCopyBufferToImage(
					cmd,
					stagingBuffer,
					vk_image,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&region
				);
			}
		}
	}
}

void VulkanTexture::copyFrom(VkCommandBuffer cmd, VulkanTexture* texture)
{
	VulkanTexture* vk_src = texture;
	VulkanTexture* vk_dst = this;

	VkImageSubresourceRange srcSubresource{};
	VkImageSubresourceRange dstSubresource{};
	srcSubresource.aspectMask = getAspectFlag(vk_src->format);
	srcSubresource.baseArrayLayer = 0;
	srcSubresource.layerCount = vk_src->layers;
	srcSubresource.baseMipLevel = 0;
	srcSubresource.levelCount = vk_src->levels;

	dstSubresource.aspectMask = getAspectFlag(vk_dst->format);
	dstSubresource.baseArrayLayer = 0;
	dstSubresource.layerCount = vk_dst->layers;
	dstSubresource.baseMipLevel = 0;
	dstSubresource.levelCount = vk_dst->levels;

	VkImageLayout srcLayout = vk_src->vk_layout;
	VkImageLayout dstLayout = vk_dst->vk_layout;

	vk_src->insertMemoryBarrier(
		cmd,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		srcSubresource,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT, // TODO color or depth, attachment or shader resource
		VK_ACCESS_TRANSFER_READ_BIT
	);
	vk_dst->insertMemoryBarrier(
		cmd,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		dstSubresource,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_READ_BIT, // TODO color or depth, attachment or shader resource
		VK_ACCESS_TRANSFER_WRITE_BIT
	);

	{
		VkImageCopy region{};
		region.srcSubresource.aspectMask = srcSubresource.aspectMask;
		region.srcSubresource.mipLevel = 0;
		region.srcSubresource.baseArrayLayer = 0;
		region.srcSubresource.layerCount = srcSubresource.layerCount;

		region.dstSubresource.aspectMask = dstSubresource.aspectMask;
		region.dstSubresource.mipLevel = 0;
		region.dstSubresource.baseArrayLayer = 0;
		region.dstSubresource.layerCount = dstSubresource.layerCount;

		region.extent.width = min(vk_src->width, vk_dst->width);
		region.extent.height = min(vk_src->height, vk_dst->height);
		region.extent.depth = 1;

		vkCmdCopyImage(cmd, vk_src->vk_image, vk_src->vk_layout, vk_dst->vk_image, vk_dst->vk_layout, 1, &region);
	}

	vk_src->insertMemoryBarrier(
		cmd,
		srcLayout,
		srcSubresource,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT // TODO color or depth, attachment or shader resource
	);
	vk_dst->insertMemoryBarrier(
		cmd,
		dstLayout,
		dstSubresource,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT // TODO color or depth, attachment or shader resource
	);
}

void VulkanTexture::blitFrom(VkCommandBuffer cmd, VulkanTexture* texture, const BlitRegion& srcRegion, const BlitRegion& dstRegion, Filter filter)
{
	VulkanTexture* vk_src = texture;
	VulkanTexture* vk_dst = this;

	VkImageSubresourceRange srcSubresource{};
	VkImageSubresourceRange dstSubresource{};
	srcSubresource.aspectMask = getAspectFlag(vk_src->format);
	srcSubresource.baseArrayLayer = srcRegion.layer;
	srcSubresource.layerCount = srcRegion.layerCount;
	srcSubresource.baseMipLevel = srcRegion.mipLevel;
	srcSubresource.levelCount = 1;

	dstSubresource.aspectMask = getAspectFlag(vk_dst->format);
	dstSubresource.baseArrayLayer = dstRegion.layer;
	dstSubresource.layerCount = dstRegion.layerCount;
	dstSubresource.baseMipLevel = dstRegion.mipLevel;
	dstSubresource.levelCount = 1;

	VkImageLayout srcLayout = vk_src->vk_layout;
	VkImageLayout dstLayout = vk_dst->vk_layout;

	vk_src->insertMemoryBarrier(
		cmd,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		srcSubresource,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT, // TODO color or depth, attachment or shader resource
		VK_ACCESS_TRANSFER_READ_BIT
	);
	vk_dst->insertMemoryBarrier(
		cmd,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		dstSubresource,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_READ_BIT, // TODO color or depth, attachment or shader resource
		VK_ACCESS_TRANSFER_WRITE_BIT
	);

	{
		VkImageBlit blit;
		blit.srcOffsets[0].x = srcRegion.x;
		blit.srcOffsets[0].y = srcRegion.y;
		blit.srcOffsets[0].z = srcRegion.z;
		blit.srcOffsets[1].x = srcRegion.w;
		blit.srcOffsets[1].y = srcRegion.h;
		blit.srcOffsets[1].z = srcRegion.d;
		blit.srcSubresource = VkImageSubresourceLayers{ srcSubresource.aspectMask, srcRegion.mipLevel, srcRegion.layer, srcRegion.layerCount };

		blit.dstOffsets[0].x = dstRegion.x;
		blit.dstOffsets[0].y = dstRegion.y;
		blit.dstOffsets[0].z = dstRegion.z;
		blit.dstOffsets[1].x = dstRegion.w;
		blit.dstOffsets[1].y = dstRegion.h;
		blit.dstOffsets[1].z = dstRegion.d;
		blit.dstSubresource = VkImageSubresourceLayers{ dstSubresource.aspectMask, dstRegion.mipLevel, dstRegion.layer, dstRegion.layerCount };

		vkCmdBlitImage(cmd,
			vk_src->vk_image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			vk_dst->vk_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&blit,
			VulkanContext::tovk(filter)
		);
	}
	{
		// Restore layout
		bool depthStencil = gfx::Texture::hasDepth(vk_dst->format) || gfx::Texture::hasStencil(vk_dst->format);
		vk_src->insertMemoryBarrier(
			cmd,
			srcLayout,
			srcSubresource,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			depthStencil ? VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT : VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_TRANSFER_READ_BIT,
			depthStencil ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT : VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
		);
	}
	{
		// Restore layout
		bool depthStencil = gfx::Texture::hasDepth(vk_dst->format) || gfx::Texture::hasStencil(vk_dst->format);
		vk_dst->insertMemoryBarrier(
			cmd,
			dstLayout,
			dstSubresource,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			depthStencil ? VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT : VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			depthStencil ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT : VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
		);
	}
}

TextureHandle VulkanGraphicDevice::createTexture(
	const char* name,
	uint32_t width, uint32_t height, uint32_t depth,
	TextureType type,
	uint32_t levels, uint32_t layers,
	TextureFormat format,
	TextureFlag flags,
	const void* const* data
)
{
	VulkanTexture* texture = m_texturePool.acquire(name, width, height, depth, type, levels, layers, format, flags);
	texture->create(m_context);

	// Upload
	if (data != nullptr && data[0] != nullptr)
	{
		texture->upload(m_context, data, 0, 0, width, height);
		VkCommandBuffer cmd = VulkanCommandList::createSingleTime(m_context.device, m_context.commandPool);
		// Generate mips
		VkImageSubresourceRange subresource = VkImageSubresourceRange{ VulkanTexture::getAspectFlag(texture->format), 0, texture->levels, 0, texture->layers };
		if (has(flags, TextureFlag::GenerateMips))
		{
			texture->generateMips(cmd); // Should be in SHADER_READ_ONLY
		}
		else
		{
			texture->transitionImageLayout(cmd,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				subresource,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
			);
		}
		VulkanCommandList::endSingleTime(m_context.device, m_context.commandPool, cmd, m_context.graphicQueue.queue);
	}
	else
	{
		VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		VkImageSubresourceRange subresource = VkImageSubresourceRange{ VulkanTexture::getAspectFlag(format), 0, levels, 0, layers };
		if (has(flags, TextureFlag::RenderTarget))
		{
			if (Texture::hasDepth(format) || Texture::hasStencil(format))
			{
				if (has(flags, TextureFlag::ShaderResource))
				{
					layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;// VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
				else
				{
					layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				}
			}
			else
			{
				if (has(flags, TextureFlag::ShaderResource))
				{
					layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
				else
				{
					layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				}
			}
		}
		VkCommandBuffer cmd = VulkanCommandList::createSingleTime(m_context.device, m_context.commandPool);
		texture->transitionImageLayout(
			cmd, layout,
			subresource,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			dstStage
		);
		VulkanCommandList::endSingleTime(m_context.device, m_context.commandPool, cmd, m_context.graphicQueue.queue);
		texture->vk_layout = layout;
	}

	return TextureHandle{ texture };
}

void VulkanGraphicDevice::upload(TextureHandle texture, const void* const* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	VulkanTexture* vk_texture = get<VulkanTexture>(texture);
	vk_texture->upload(m_context, data, x, y, width, height);
	// Transition back to shader read only
	VkImageSubresourceRange subresource{ VulkanTexture::getAspectFlag(vk_texture->format), 0, vk_texture->levels, 0, vk_texture->layers };
	VkCommandBuffer cmd = VulkanCommandList::createSingleTime(m_context.device, m_context.commandPool);
	vk_texture->transitionImageLayout(
		cmd,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		subresource,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
	);
	VulkanCommandList::endSingleTime(m_context.device, m_context.commandPool, cmd, m_context.graphicQueue.queue);
}

void VulkanGraphicDevice::download(TextureHandle texture, void* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t layer)
{
	AKA_NOT_IMPLEMENTED;
}

void VulkanGraphicDevice::copy(TextureHandle lhs, TextureHandle rhs)
{
	VulkanTexture* vk_src = get<VulkanTexture>(lhs);
	VulkanTexture* vk_dst = get<VulkanTexture>(rhs);

	VkCommandBuffer cmd = VulkanCommandList::createSingleTime(m_context.device, m_context.commandPool);
	vk_dst->copyFrom(cmd, vk_src);
	VulkanCommandList::endSingleTime(m_context.device, m_context.commandPool, cmd, m_context.graphicQueue.queue);
}

void VulkanGraphicDevice::destroy(TextureHandle texture)
{
	if (texture.data == nullptr) return;

	VulkanTexture* vk_texture = get<VulkanTexture>(texture);
	vkDestroyImageView(m_context.device, vk_texture->vk_view, nullptr);
	vk_texture->vk_view = VK_NULL_HANDLE;
	if (vk_texture->vk_memory != 0) // If no memory used, image not allocated here (swapchain)
	{
		vkFreeMemory(m_context.device, vk_texture->vk_memory, nullptr);
		vkDestroyImage(m_context.device, vk_texture->vk_image, nullptr);
		vk_texture->vk_memory = VK_NULL_HANDLE;
		vk_texture->vk_image = VK_NULL_HANDLE;
	}
	m_texturePool.release(vk_texture);
}

VulkanTexture::VulkanTexture(const char* name, uint32_t width, uint32_t height, uint32_t depth, TextureType type, uint32_t levels, uint32_t layers, TextureFormat format, TextureFlag flags) :
	Texture(name, width, height, depth, type, levels, layers, format, flags),
	vk_image(VK_NULL_HANDLE),
	vk_memory(VK_NULL_HANDLE),
	vk_view(VK_NULL_HANDLE),
	vk_layout(VK_IMAGE_LAYOUT_UNDEFINED)
{
}

void VulkanTexture::create(VulkanContext& context)
{
	AKA_ASSERT(layers > 0, "Should have at least 1 layer");
	AKA_ASSERT(levels > 0, "Should have at least 1 mips");
	VkFormat vk_format = VulkanContext::tovk(format);
	VkImageTiling vk_tiling = VK_IMAGE_TILING_OPTIMAL; // TODO control this (flag dependent ? staging ?)
	VkImageUsageFlags vk_usage = 0;
	VkImageCreateFlags vk_flags = 0;
	VkImageAspectFlags vk_aspect = VulkanTexture::getAspectFlag(format);
	VkImageViewType vk_type = VK_IMAGE_VIEW_TYPE_2D;
	switch (type)
	{
	case TextureType::Texture2D:
		vk_type = VK_IMAGE_VIEW_TYPE_2D;
		break;
	case TextureType::TextureCubeMap:
		AKA_ASSERT(width == height, "Cubemap need to be squared.");
		vk_type = VK_IMAGE_VIEW_TYPE_CUBE;
		vk_flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		break;
	case TextureType::Texture2DArray:
		vk_type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		break;
	default:
		AKA_ASSERT(false, "Invalid type.");
		break;
	}
	if (has(flags, TextureFlag::RenderTarget))
	{
		if (Texture::hasDepth(format))
			vk_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else
			vk_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (has(flags, TextureFlag::ShaderResource))
	{
		vk_usage |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; // Need to upload to it.
	}
	if (has(flags, TextureFlag::Storage))
	{
		vk_usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	}

	{
		// TODO add flags for transfer SRC and DST
		vk_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		vk_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	vk_image = VulkanTexture::createVkImage(context.device, width, height, levels, layers, vk_format, vk_tiling, vk_usage, vk_flags);

	// Create memory
	VkMemoryRequirements memRequirements{};
	vkGetImageMemoryRequirements(context.device, vk_image, &memRequirements);

	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	// TODO add settings to set host coherent
	// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = context.findMemoryType(memRequirements.memoryTypeBits, properties);

	VK_CHECK_RESULT(vkAllocateMemory(context.device, &allocInfo, nullptr, &vk_memory));

	VK_CHECK_RESULT(vkBindImageMemory(context.device, vk_image, vk_memory, 0));

	// Create View
	// View must only have one aspect flag
	VkImageAspectFlags vk_viewAspect = VK_IMAGE_ASPECT_COLOR_BIT;
	if (Texture::hasDepth(format))
		vk_viewAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	else if (Texture::hasStencil(format))
		vk_viewAspect = VK_IMAGE_ASPECT_STENCIL_BIT;
	vk_view = VulkanTexture::createVkImageView(context.device, vk_image, vk_type, vk_format, vk_viewAspect, levels, layers);

	setDebugName(context.device, vk_image, "VkImage_", name);
	setDebugName(context.device, vk_view, "VkImageView_", name);
	setDebugName(context.device, vk_memory, "VkDeviceMemory_", name);
}

void VulkanTexture::destroy(VulkanContext& context)
{
	vkDestroyImageView(context.device, vk_view, nullptr);
	vk_view = VK_NULL_HANDLE;
	if (vk_memory != 0) // If no memory used, image not allocated here (swapchain)
	{
		vkFreeMemory(context.device, vk_memory, nullptr);
		vkDestroyImage(context.device, vk_image, nullptr);
		vk_memory = VK_NULL_HANDLE;
		vk_image = VK_NULL_HANDLE;
	}
}

void VulkanTexture::upload(VulkanContext& context, const void* const* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	AKA_ASSERT(x == 0 && y == 0 && width == this->width && height == this->height, "Subregion upload not supported yet. Should specify which layer / level to upload as well.");

	if (data == nullptr || data[0] == nullptr)
		return;

	// Create staging buffer
	VkDeviceSize imageSize = this->width * this->height * Texture::size(this->format);
	VkDeviceSize bufferSize = imageSize * this->layers;
	VkBuffer stagingBuffer = VulkanBuffer::createVkBuffer(
		context.device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT
	);
	VkDeviceMemory stagingBufferMemory = VulkanBuffer::createVkDeviceMemory(
		context.device,
		context.physicalDevice,
		stagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	// Upload to staging buffer
	void* stagingData;
	vkMapMemory(context.device, stagingBufferMemory, 0, bufferSize, 0, &stagingData);
	for (uint32_t iLayer = 0; iLayer < this->layers; iLayer++)
	{
		void* offset = static_cast<char*>(stagingData) + imageSize * iLayer;
		memcpy(offset, data[iLayer], static_cast<size_t>(imageSize));
	}
	vkUnmapMemory(context.device, stagingBufferMemory);

	// Copy buffer to image
	VkCommandBuffer cmd = VulkanCommandList::createSingleTime(context.device, context.commandPool);
	VkImageSubresourceRange subresource = VkImageSubresourceRange{ VulkanTexture::getAspectFlag(this->format), 0, this->levels, 0, this->layers };
	transitionImageLayout(cmd,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresource,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT
	);
	copyBufferToImage(cmd, stagingBuffer);

	VulkanCommandList::endSingleTime(context.device, context.commandPool, cmd, context.graphicQueue.queue);

	// Free staging buffer
	vkFreeMemory(context.device, stagingBufferMemory, nullptr);
	vkDestroyBuffer(context.device, stagingBuffer, nullptr);
}

VkImage VulkanTexture::createVkImage(VkDevice device, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = layers;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.flags = flags;

	VkImage image;
	VK_CHECK_RESULT(vkCreateImage(device, &imageInfo, nullptr, &image));
	return image;
}

VkImageView VulkanTexture::createVkImageView(VkDevice device, VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspect, uint32_t mipLevels, uint32_t layers, uint32_t baseMips, uint32_t baseLayer)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = type;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspect;
	viewInfo.subresourceRange.baseMipLevel = baseMips;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = baseLayer;
	viewInfo.subresourceRange.layerCount = layers;

	VkImageView view;
	VK_CHECK_RESULT(vkCreateImageView(device, &viewInfo, nullptr, &view));
	return view;
}

void VulkanTexture::insertMemoryBarrier(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subResource, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkAccessFlags srcAccess, VkAccessFlags dstAccess)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = subResource;
	barrier.srcAccessMask = srcAccess;
	barrier.dstAccessMask = dstAccess;

	vkCmdPipelineBarrier(
		cmd,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}

void VulkanTexture::generateMips(VkCommandBuffer commandBuffer)
{
	// Check if image format supports linear blitting
	/*VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!"); 
	}*/

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = vk_image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = getAspectFlag(format);
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = width;
	int32_t mipHeight = height;

	// TODO cubemap mips
	AKA_ASSERT(layers == 1, "More than one layer unsupported for now");
	//for (uint32_t iLayer = 1; iLayer < layers; iLayer++)
	for (uint32_t iLevel = 1; iLevel < levels; iLevel++)
	{
		barrier.subresourceRange.baseMipLevel = iLevel - 1;
		barrier.oldLayout = vk_layout;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = getAspectFlag(format);
		blit.srcSubresource.mipLevel = iLevel - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = getAspectFlag(format);
		blit.dstSubresource.mipLevel = iLevel;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(
			commandBuffer,
			vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR
		);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		if (mipWidth > 1) 
			mipWidth /= 2;
		if (mipHeight > 1) 
			mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = levels - 1;
	barrier.oldLayout = vk_layout;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	vk_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

};
};

#endif