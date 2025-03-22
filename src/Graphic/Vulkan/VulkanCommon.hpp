#pragma once

#include <vulkan/vulkan.h>

#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Graphic/Pipeline.h>
#include <Aka/Graphic/Sampler.h>

#include "VulkanDebug.h"

namespace aka {
namespace gfx {

namespace vk {
template <typename T, typename ...Args>
void setDebugName(VkDevice device, T handle, Args ...args)
{
	AKA_ASSERT(handle != VK_NULL_HANDLE, "Invalid handle");
	static_assert(VulkanTypeTrait<T>::debugType != VK_OBJECT_TYPE_UNKNOWN);
	String fmt_name = String::from(args...);
	VkDebugUtilsObjectNameInfoEXT info{};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	info.objectHandle = (uint64_t)handle;
	info.objectType = VulkanTypeTrait<T>::debugType;
	info.pObjectName = fmt_name.cstr();
	VK_CHECK_RESULT(vkSetDebugUtilsObjectNameEXT(device, &info));
}

VkFormat convert(TextureFormat format);
VkFilter convert(Filter filter);
VkIndexType convert(IndexFormat format);
VkBufferUsageFlags convert(BufferType type);
VkMemoryPropertyFlags convert(BufferUsage type);
VkImageLayout convert(ResourceAccessType type, TextureFormat format);
VkAttachmentLoadOp convert(AttachmentLoadOp loadOp);
VkAttachmentStoreOp convert(AttachmentStoreOp storeOp);
VkDescriptorType convert(ShaderBindingType type);
VkShaderStageFlags convert(ShaderMask shaderType);
}
// Dump it all in gfx namespace for now.
using namespace vk;
}
}