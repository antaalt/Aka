#include "VulkanCommon.hpp"

namespace aka {
namespace gfx {

namespace vk {

VkFormat convert(TextureFormat format)
{
	switch (format)
	{
	default:
		AKA_ASSERT(false, "Invalid texture format");
		return VK_FORMAT_UNDEFINED;
	case TextureFormat::Swapchain:
		return VK_FORMAT_B8G8R8A8_UNORM; // TODO: retrieve dynamically
	case TextureFormat::R8:
	case TextureFormat::R8U:
		return VK_FORMAT_R8_UNORM; // VK_FORMAT_R8_UINT
	case TextureFormat::R16:
	case TextureFormat::R16U:
		return VK_FORMAT_R16_UNORM;
	case TextureFormat::R16F:
		return VK_FORMAT_R16_SFLOAT;
	case TextureFormat::R32F:
		return VK_FORMAT_R32_SFLOAT;
	case TextureFormat::RG8:
	case TextureFormat::RG8U:
		return VK_FORMAT_R8G8_UNORM;
	case TextureFormat::RG16:
	case TextureFormat::RG16U:
		return VK_FORMAT_R16G16_UNORM;
	case TextureFormat::RG16F:
		return VK_FORMAT_R16G16_SFLOAT;
	case TextureFormat::RG32F:
		return VK_FORMAT_R32G32_SFLOAT;
	case TextureFormat::RGB8:
	case TextureFormat::RGB8U:
		return VK_FORMAT_R8G8B8_UNORM;
	case TextureFormat::RGB16:
	case TextureFormat::RGB16U:
		return VK_FORMAT_R16G16B16_UNORM;
	case TextureFormat::RGB16F:
		return VK_FORMAT_R16G16B16_SFLOAT;
	case TextureFormat::RGB32F:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case TextureFormat::RGBA8:
	case TextureFormat::RGBA8U:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case TextureFormat::RGBA16:
	case TextureFormat::RGBA16U:
		return VK_FORMAT_R16G16B16A16_UNORM;
	case TextureFormat::RGBA16F:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case TextureFormat::RGBA32F:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case TextureFormat::BGRA:
	case TextureFormat::BGRA8:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case TextureFormat::Depth16:
		return VK_FORMAT_D16_UNORM;
	case TextureFormat::Depth24:
		return VK_FORMAT_D24_UNORM_S8_UINT;
	case TextureFormat::Depth:
	case TextureFormat::Depth32:
	case TextureFormat::Depth32F:
		return VK_FORMAT_D32_SFLOAT;
	case TextureFormat::Depth0Stencil8:
		return VK_FORMAT_S8_UINT;
	case TextureFormat::DepthStencil:
	case TextureFormat::Depth24Stencil8:
		return VK_FORMAT_D24_UNORM_S8_UINT;
	case TextureFormat::Depth32FStencil8:
		return VK_FORMAT_D32_SFLOAT_S8_UINT;
	}
	return VK_FORMAT_UNDEFINED;
}

VkFilter convert(Filter filter)
{
	switch (filter)
	{
	default:
		AKA_ASSERT(false, "Invalid filter");
		[[fallthrough]];
	case Filter::Nearest:
		return VK_FILTER_NEAREST;
	case Filter::Linear:
		return VK_FILTER_LINEAR;
	}
}

VkIndexType convert(IndexFormat format)
{
	switch (format)
	{
	case IndexFormat::UnsignedByte:
		return VK_INDEX_TYPE_UINT8_EXT;
	case IndexFormat::UnsignedShort:
		return VK_INDEX_TYPE_UINT16;
	default:
		AKA_ASSERT(false, "Invalid index format");
		[[fallthrough]];
	case IndexFormat::UnsignedInt:
		return VK_INDEX_TYPE_UINT32;
	}
}

VkBufferUsageFlags convert(BufferType type)
{
	VkBufferUsageFlags flags = 0;
	if (has(type, BufferType::Vertex))
		flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	if (has(type, BufferType::Index))
		flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	if (has(type, BufferType::Uniform))
		flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	if (has(type, BufferType::Storage))
		flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	if (has(type, BufferType::Indirect))
		flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	return flags;
}

// Resource usage	| Default	| Dynamic	| Immutable	| Staging
// GPU-Read			| Yes		| Yes		| Yes		| Yes
// GPU-Write		| Yes		| --		| --		| Yes
// CPU-Read			| --		| --		| --		| Yes
// CPU-Write		| --		| Yes		| --		| Yes
VkMemoryPropertyFlags convert(BufferUsage type)
{
	// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT = can be mapped
	// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT = needed for map
	switch (type)
	{
	case BufferUsage::Default:
		return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	case BufferUsage::Dynamic: // Support map
		// Should not be only host coherent
		return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	case BufferUsage::Staging:
		return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	case BufferUsage::Immutable:
		return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	default:
		return 0;
	}
}
//#define DEPTH_AND_STENCIL_SEPARATELY 1 // separateDepthStencilLayouts
VkImageLayout convert(ResourceAccessType type, TextureFormat format)
{
	bool depth = Texture::hasDepth(format);
	bool stencil = Texture::hasStencil(format);
	// TODO should handle these somehow... (need DEPTH_AND_STENCIL_SEPARATELY)
	// VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
	// VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL
	switch (type)
	{
	default:
		AKA_ASSERT(false, "Invalid attachment load op");
		[[fallthrough]];
	case ResourceAccessType::Undefined:
		return VK_IMAGE_LAYOUT_UNDEFINED;
	case ResourceAccessType::Resource:
		if (depth && stencil) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
#ifdef DEPTH_AND_STENCIL_SEPARATELY
		if (depth) return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
		else if (stencil) return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
#endif
		else return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case ResourceAccessType::Attachment:
#ifdef DEPTH_AND_STENCIL_SEPARATELY
		if (depth && stencil) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		else if (depth) return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		else if (stencil) return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
#else
		if (depth || stencil) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
#endif
		else return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case ResourceAccessType::Storage:
		return VK_IMAGE_LAYOUT_GENERAL;
	case ResourceAccessType::CopySRC:
		return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	case ResourceAccessType::CopyDST:
		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	case ResourceAccessType::Present:
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
}
VkAttachmentLoadOp convert(AttachmentLoadOp loadOp)
{
	switch (loadOp)
	{
	default:
		AKA_ASSERT(false, "Invalid attachment load op");
		[[fallthrough]];
	case AttachmentLoadOp::Clear:
		return VK_ATTACHMENT_LOAD_OP_CLEAR;
	case AttachmentLoadOp::Load:
		return VK_ATTACHMENT_LOAD_OP_LOAD;
	case AttachmentLoadOp::DontCare:
		return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
}
VkAttachmentStoreOp convert(AttachmentStoreOp loadOp)
{
	switch (loadOp)
	{
	default:
		AKA_ASSERT(false, "Invalid attachment store op");
		[[fallthrough]];
	case AttachmentStoreOp::Store:
		return VK_ATTACHMENT_STORE_OP_STORE;
	case AttachmentStoreOp::DontCare:
		return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
}
}
}
}