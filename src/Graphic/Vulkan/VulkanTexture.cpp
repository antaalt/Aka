#if defined(AKA_USE_VULKAN)

#include "VulkanTexture.h"
#include "VulkanGraphicDevice.h"

namespace aka {
namespace gfx {

VkImageAspectFlags VulkanTexture::getAspectFlag(TextureFormat format)
{
	// View must only have one aspect flag
	if (Texture::isColor(format))
		return VK_IMAGE_ASPECT_COLOR_BIT;
	else if (Texture::isDepthStencil(format))
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	else if (Texture::isDepth(format))
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	else if (Texture::isStencil(format))
		return VK_IMAGE_ASPECT_STENCIL_BIT;
	AKA_UNREACHABLE;
	return VK_IMAGE_ASPECT_COLOR_BIT;
}

// https://themaister.net/blog/2019/08/14/yet-another-blog-explaining-vulkan-synchronization/
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineStageFlagBits.html
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAccessFlagBits.html
VkAccessFlags accessFlagForLayout(ResourceAccessType type, TextureFormat format, bool src)
{
	bool depth = Texture::hasDepth(format);
	bool stencil = Texture::hasStencil(format);
	switch (type)
	{
	default:
		AKA_ASSERT(false, "Invalid access type");
		[[fallthrough]];
	case ResourceAccessType::Undefined:
#if defined(VK_VERSION_1_3)
		return VK_ACCESS_NONE;
#else
		return 0;
#endif
	case ResourceAccessType::Resource: // Read only
		return VK_ACCESS_SHADER_READ_BIT; // Only for shader read
		//return VK_ACCESS_MEMORY_READ_BIT; // Most general purpose
		//return VK_ACCESS_HOST_READ_BIT; // access on host (VK_PIPELINE_STAGE_HOST_BIT)
	case ResourceAccessType::Attachment:
		//if (depth || stencil) return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT; // VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT or VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT 
		//else return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT; // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 
		if (depth || stencil) return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		else return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	case ResourceAccessType::Storage:
		return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	case ResourceAccessType::CopySRC:
		return VK_ACCESS_TRANSFER_READ_BIT; // VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT 
	case ResourceAccessType::CopyDST:
		return VK_ACCESS_TRANSFER_WRITE_BIT; // VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT 
	case ResourceAccessType::Present:
		return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	}
}

VkPipelineStageFlags pipelineStageForLayout(ResourceAccessType type, TextureFormat format, bool src)
{
	bool depth = Texture::hasDepth(format);
	bool stencil = Texture::hasStencil(format);
	switch (type)
	{
	default:
		AKA_ASSERT(false, "Invalid access type");
		[[fallthrough]];
	case ResourceAccessType::Undefined:
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	case ResourceAccessType::Resource:
		return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
			| VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
			| VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
#if 0 // Tesselation support...
			| VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
			| VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
#endif
			;
	case ResourceAccessType::Attachment:
		if (depth || stencil) return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT; // TODO: VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
		else return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	case ResourceAccessType::Storage:
		return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
			| VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
			| VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
#if 0 // Tesselation support...
			| VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
			| VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
#endif
			;
	case ResourceAccessType::CopySRC:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	case ResourceAccessType::CopyDST:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	case ResourceAccessType::Present:
		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
}

void VulkanTexture::transitionImageLayout(VkCommandBuffer cmd, VkImage image, ResourceAccessType oldLayout, ResourceAccessType newLayout, TextureFormat format, uint32_t level,	uint32_t levelCount, uint32_t layer, uint32_t layerCount)
{
	if (oldLayout == newLayout)
		return; // TODO barrier
	VulkanTexture::insertMemoryBarrier(
		cmd, 
		image,
		VulkanContext::tovk(oldLayout, format), 
		VulkanContext::tovk(newLayout, format),
		VkImageSubresourceRange{ VulkanTexture::getAspectFlag(format), level, levelCount, layer, layerCount },
		pipelineStageForLayout(oldLayout, format, true),
		pipelineStageForLayout(newLayout, format, false),
		accessFlagForLayout(oldLayout, format, true),
		accessFlagForLayout(newLayout, format, false)
	);
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
				region.imageSubresource.layerCount = 1; // TODO: all layers

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

	VkImageCopy region{};
	region.srcSubresource.aspectMask = getAspectFlag(vk_src->format);
	region.srcSubresource.mipLevel = 0;
	region.srcSubresource.baseArrayLayer = 0;
	region.srcSubresource.layerCount = vk_src->layers;

	region.dstSubresource.aspectMask = getAspectFlag(vk_dst->format);
	region.dstSubresource.mipLevel = 0;
	region.dstSubresource.baseArrayLayer = 0;
	region.dstSubresource.layerCount = vk_dst->layers;

	region.extent.width = min(vk_src->width, vk_dst->width);
	region.extent.height = min(vk_src->height, vk_dst->height);
	region.extent.depth = 1;

	vkCmdCopyImage(cmd, vk_src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vk_dst->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VulkanTexture::blitFrom(VkCommandBuffer cmd, VulkanTexture* texture, const BlitRegion& srcRegion, const BlitRegion& dstRegion, Filter filter)
{
	VulkanTexture* vk_src = texture;
	VulkanTexture* vk_dst = this;

	VkImageBlit blit;
	blit.srcOffsets[0].x = srcRegion.x;
	blit.srcOffsets[0].y = srcRegion.y;
	blit.srcOffsets[0].z = srcRegion.z;
	blit.srcOffsets[1].x = srcRegion.w;
	blit.srcOffsets[1].y = srcRegion.h;
	blit.srcOffsets[1].z = srcRegion.d;
	blit.srcSubresource = VkImageSubresourceLayers{ getAspectFlag(vk_src->format), srcRegion.mipLevel, srcRegion.layer, srcRegion.layerCount };

	blit.dstOffsets[0].x = dstRegion.x;
	blit.dstOffsets[0].y = dstRegion.y;
	blit.dstOffsets[0].z = dstRegion.z;
	blit.dstOffsets[1].x = dstRegion.w;
	blit.dstOffsets[1].y = dstRegion.h;
	blit.dstOffsets[1].z = dstRegion.d;
	blit.dstSubresource = VkImageSubresourceLayers{ getAspectFlag(vk_dst->format), dstRegion.mipLevel, dstRegion.layer, dstRegion.layerCount };

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

ResourceAccessType getInitialResourceAccessType(TextureFormat format, TextureUsage flags)
{
	if (has(flags, TextureUsage::Storage))
	{
		return ResourceAccessType::Storage;
	}
	else if (has(flags, TextureUsage::ShaderResource))
	{
		return ResourceAccessType::Resource;
	}
	else if (has(flags, TextureUsage::RenderTarget))
	{
		return ResourceAccessType::Attachment;
	}
	else
	{
		AKA_UNREACHABLE;
		return ResourceAccessType::Undefined;
	}
}

TextureHandle VulkanGraphicDevice::createTexture(
	const char* name,
	uint32_t width, uint32_t height, uint32_t depth,
	TextureType type,
	uint32_t levels, uint32_t layers,
	TextureFormat format,
	TextureUsage flags,
	const void* const* data
)
{
	VulkanTexture* vk_texture = m_texturePool.acquire(name, width, height, depth, type, levels, layers, format, flags);
	vk_texture->create(this);

	TextureHandle handle = TextureHandle{ vk_texture };

	ResourceAccessType finalAccessType = getInitialResourceAccessType(format, flags);
	VkImageLayout finalLayout = VulkanContext::tovk(finalAccessType, format);

	// Upload
	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
	VkImageSubresourceRange subResource = VkImageSubresourceRange{ VulkanTexture::getAspectFlag(format), 0, levels, 0, layers };
	VkCommandBuffer cmd = VulkanCommandList::createSingleTime("Uploading textures & mips", getVkDevice(), getVkCommandPool(QueueType::Graphic));
	if (data != nullptr && data[0] != nullptr)
	{
		// Create staging buffer
		VkDeviceSize imageSize = vk_texture->width * vk_texture->height * Texture::size(vk_texture->format);
		VkDeviceSize bufferSize = imageSize * vk_texture->layers;
		stagingBuffer = VulkanBuffer::createVkBuffer(
			getVkDevice(),
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT
		);
		stagingBufferMemory = VulkanBuffer::createVkDeviceMemory(
			getVkDevice(),
			getVkPhysicalDevice(),
			stagingBuffer,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		// Upload to staging buffer
		void* stagingData;
		vkMapMemory(getVkDevice(), stagingBufferMemory, 0, bufferSize, 0, &stagingData);
		for (uint32_t iLayer = 0; iLayer < vk_texture->layers; iLayer++)
		{
			void* offset = static_cast<char*>(stagingData) + imageSize * iLayer;
			memcpy(offset, data[iLayer], static_cast<size_t>(imageSize));
		}
		vkUnmapMemory(getVkDevice(), stagingBufferMemory);

		// Prepare for transfer
		VulkanTexture::transitionImageLayout(cmd,
			vk_texture->vk_image,
			ResourceAccessType::Undefined,
			ResourceAccessType::CopyDST,
			format,
			0U,
			vk_texture->levels,
			0U,
			vk_texture->layers
		);
		// Copy buffer to image
		vk_texture->copyBufferToImage(cmd, stagingBuffer);

		// Generate mips
		if (has(flags, TextureUsage::GenerateMips))
		{
			vk_texture->generateMips(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, finalLayout);
		}
		else
		{
			VulkanTexture::transitionImageLayout(cmd,
				vk_texture->vk_image,
				ResourceAccessType::CopyDST,
				finalAccessType,
				format
			);
		}
	}
	else
	{
		VulkanTexture::transitionImageLayout(cmd,
			vk_texture->vk_image,
			ResourceAccessType::Undefined,
			finalAccessType,
			format
		);
	}
	VulkanCommandList::endSingleTime(getVkDevice(), getVkCommandPool(QueueType::Graphic), cmd, getVkQueue(QueueType::Graphic));

	// Free staging buffer
	vkFreeMemory(getVkDevice(), stagingBufferMemory, getVkAllocator());
	vkDestroyBuffer(getVkDevice(), stagingBuffer, getVkAllocator());

	return handle;
}

void VulkanGraphicDevice::upload(TextureHandle texture, const void* const* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	// TODO split staging buffer
	VulkanTexture* vk_texture = getVk<VulkanTexture>(texture);
	vk_texture->upload(this, data, x, y, width, height);
}

void VulkanGraphicDevice::download(TextureHandle texture, void* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t layer)
{
	AKA_NOT_IMPLEMENTED;
}

void VulkanGraphicDevice::copy(TextureHandle src, TextureHandle dst)
{
	VulkanTexture* vk_src = getVk<VulkanTexture>(src);
	VulkanTexture* vk_dst = getVk<VulkanTexture>(dst);

	VkCommandBuffer cmd = VulkanCommandList::createSingleTime("Copying textures", getVkDevice(), getVkCommandPool(QueueType::Graphic));
	vk_dst->copyFrom(cmd, vk_src);
	VulkanCommandList::endSingleTime(getVkDevice(), getVkCommandPool(QueueType::Graphic), cmd, getVkQueue(QueueType::Graphic));
}

void VulkanGraphicDevice::destroy(TextureHandle texture)
{
	if (texture == TextureHandle::null) return;

	VulkanTexture* vk_texture = getVk<VulkanTexture>(texture);
	vk_texture->destroy(this);

	m_texturePool.release(vk_texture);
}

void VulkanGraphicDevice::transition(TextureHandle texture, ResourceAccessType src, ResourceAccessType dst)
{
	VulkanTexture* vk_texture = getVk<VulkanTexture>(texture);
	VkCommandBuffer cmd = VulkanCommandList::createSingleTime("Transition resource", getVkDevice(), getVkCommandPool(QueueType::Graphic));
	VulkanTexture::transitionImageLayout(
		cmd,
		vk_texture->vk_image, 
		src,
		dst,
		vk_texture->format,
		0, vk_texture->levels, 
		0, vk_texture->layers
	);
	VulkanCommandList::endSingleTime(getVkDevice(), getVkCommandPool(QueueType::Graphic), cmd, getVkQueue(QueueType::Graphic));
}

const Texture* VulkanGraphicDevice::get(TextureHandle handle)
{
	return static_cast<const Texture*>(handle.__data);
}

VkImageViewType getVkImageViewType(TextureType type, VkImageCreateFlags& vk_flags, uint32_t width, uint32_t height)
{
	VkImageViewType vk_viewType = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
	switch (type)
	{
	case TextureType::Texture2D:
		vk_viewType = VK_IMAGE_VIEW_TYPE_2D;
		break;
	case TextureType::TextureCubeMap:
		AKA_ASSERT(width == height, "Cubemap need to be squared.");
		vk_viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		vk_flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		break;
	case TextureType::Texture2DArray:
		vk_viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		break;
	default:
		AKA_ASSERT(false, "Invalid type.");
		break;
	}
	return vk_viewType;
}

VulkanTexture::VulkanTexture(const char* name, uint32_t width, uint32_t height, uint32_t depth, TextureType type, uint32_t levels, uint32_t layers, TextureFormat format, TextureUsage flags) :
	Texture(name, width, height, depth, type, levels, layers, format, flags),
	vk_image(VK_NULL_HANDLE),
	vk_memory(VK_NULL_HANDLE),
	vk_view(layers * levels + 1, VK_NULL_HANDLE)
{
}

void VulkanTexture::create(VulkanGraphicDevice* device)
{
	AKA_ASSERT(layers > 0, "Should have at least 1 layer");
	AKA_ASSERT(levels > 0, "Should have at least 1 mips");
	VkFormat vk_format = VulkanContext::tovk(format);
	VkImageTiling vk_tiling = VK_IMAGE_TILING_OPTIMAL; // TODO control this (flag dependent ? staging ?)
	VkImageUsageFlags vk_usage = 0;
	VkImageCreateFlags vk_flags = 0;
	VkImageAspectFlags vk_aspect = VulkanTexture::getAspectFlag(format);
	VkImageViewType vk_viewType = getVkImageViewType(type, vk_flags, width, height);

	if (has(flags, TextureUsage::RenderTarget))
	{
		if (Texture::hasDepth(format))
			vk_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else
			vk_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (has(flags, TextureUsage::ShaderResource))
	{
		vk_usage |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; // Need to upload to it.
	}
	if (has(flags, TextureUsage::Storage))
	{
		vk_usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	}

	{
		// TODO add flags for transfer SRC and DST
		vk_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		vk_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	vk_image = VulkanTexture::createVkImage(device->getVkDevice(), width, height, levels, layers, vk_format, vk_tiling, vk_usage, vk_flags);

	// Create memory
	VkMemoryRequirements memRequirements{};
	vkGetImageMemoryRequirements(device->getVkDevice(), vk_image, &memRequirements);

	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	// TODO add settings to set host coherent
	// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, properties);

	VK_CHECK_RESULT(vkAllocateMemory(device->getVkDevice(), &allocInfo, getVkAllocator(), &vk_memory));

	VK_CHECK_RESULT(vkBindImageMemory(device->getVkDevice(), vk_image, vk_memory, 0));

	// Create main view.
	vk_view[0] = VulkanTexture::createVkImageView(device->getVkDevice(), vk_image, vk_viewType, vk_format, VulkanTexture::getAspectFlag(format), levels, layers);

	setDebugName(device->getVkDevice(), vk_image, name, "Image");
	setDebugName(device->getVkDevice(), vk_view[0], name, "ImageMainView");
	setDebugName(device->getVkDevice(), vk_memory, name, "DeviceMemory");
}

void VulkanTexture::destroy(VulkanGraphicDevice* device)
{
	for (VkImageView& view : vk_view)
	{
		vkDestroyImageView(device->getVkDevice(), view, getVkAllocator());
		view = VK_NULL_HANDLE;
	}
	if (vk_memory != 0) // If no memory used, image not allocated here (swapchain)
	{
		vkFreeMemory(device->getVkDevice(), vk_memory, getVkAllocator());
		vkDestroyImage(device->getVkDevice(), vk_image, getVkAllocator());
		vk_memory = VK_NULL_HANDLE;
		vk_image = VK_NULL_HANDLE;
	}
}

void VulkanTexture::upload(VulkanGraphicDevice* device, const void* const* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	AKA_ASSERT(x == 0 && y == 0 && width == this->width && height == this->height, "Subregion upload not supported yet. Should specify which layer / level to upload as well.");

	if (data == nullptr || data[0] == nullptr)
		return;

	// Create staging buffer
	VkDeviceSize imageSize = this->width * this->height * Texture::size(this->format);
	VkDeviceSize bufferSize = imageSize * this->layers;
	VkBuffer stagingBuffer = VulkanBuffer::createVkBuffer(
		device->getVkDevice(),
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT
	);
	VkDeviceMemory stagingBufferMemory = VulkanBuffer::createVkDeviceMemory(
		device->getVkDevice(),
		device->getVkPhysicalDevice(),
		stagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	// Upload to staging buffer
	void* stagingData;
	vkMapMemory(device->getVkDevice(), stagingBufferMemory, 0, bufferSize, 0, &stagingData);
	for (uint32_t iLayer = 0; iLayer < this->layers; iLayer++)
	{
		void* offset = static_cast<char*>(stagingData) + imageSize * iLayer;
		memcpy(offset, data[iLayer], static_cast<size_t>(imageSize));
	}
	vkUnmapMemory(device->getVkDevice(), stagingBufferMemory);

	// Copy buffer to image
	VkCommandBuffer cmd = VulkanCommandList::createSingleTime("Upload texture", device->getVkDevice(), device->getVkCommandPool(QueueType::Graphic));
	copyBufferToImage(cmd, stagingBuffer);
	VulkanCommandList::endSingleTime(device->getVkDevice(), device->getVkCommandPool(QueueType::Graphic), cmd, device->getVkQueue(QueueType::Graphic));

	// Free staging buffer
	vkFreeMemory(device->getVkDevice(), stagingBufferMemory, getVkAllocator());
	vkDestroyBuffer(device->getVkDevice(), stagingBuffer, getVkAllocator());
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
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // VK_IMAGE_LAYOUT_PREINITIALIZED
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.flags = flags;

	VkImage image;
	VK_CHECK_RESULT(vkCreateImage(device, &imageInfo, getVkAllocator(), &image));
	return image;
}

VkImageView VulkanTexture::createVkImageView(VkDevice device, VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspect, uint32_t mipCount, uint32_t layerCount, uint32_t baseMips, uint32_t baseLayer)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = type;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspect;
	viewInfo.subresourceRange.baseMipLevel = baseMips;
	viewInfo.subresourceRange.levelCount = mipCount;
	viewInfo.subresourceRange.baseArrayLayer = baseLayer;
	viewInfo.subresourceRange.layerCount = layerCount;

	VkImageView view;
	VK_CHECK_RESULT(vkCreateImageView(device, &viewInfo, getVkAllocator(), &view));
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

VkImageView VulkanTexture::getMainImageView() const
{
	return vk_view[0]; // Main view always defined.
}

VkImageView VulkanTexture::getImageView(VulkanGraphicDevice* device, uint32_t layer, uint32_t mipLevel)
{
	uint32_t index = layer * mipLevel + mipLevel + 1;
	VkImageCreateFlags vk_flags;
	if (vk_view[index] == VK_NULL_HANDLE)
	{
		vk_view[index] = createVkImageView(device->getVkDevice(), vk_image, getVkImageViewType(type, vk_flags, width, height), VulkanContext::tovk(format), VulkanTexture::getAspectFlag(format), 1, 1, mipLevel, layer);
		setDebugName(device->getVkDevice(), vk_view[index], name, "ImageView", index);
	}
	return vk_view[index];
}

void VulkanTexture::generateMips(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout)
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
		barrier.oldLayout = oldLayout;
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
		barrier.newLayout = newLayout;
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
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	//vk_layout = newLayout;
}

};
};

#endif