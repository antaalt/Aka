#pragma once

#if defined(AKA_USE_VULKAN)


#include <vulkan/vulkan.h>

#define VK_DEBUG_ALLOCATION

#define VK_CHECK_RESULT(result)				\
{											\
	VkResult res = (result);				\
	if (VK_SUCCESS != res) {				\
		char buffer[256];					\
		snprintf(							\
			buffer,							\
			256,							\
			"%s (%s at %s:%d)",				\
			aka::gfx::getVkErrorString(res),\
			AKA_STRINGIFY(result),			\
			__FILE__,						\
			__LINE__						\
		);									\
		::aka::Logger::error(buffer);       \
		AKA_DEBUG_BREAK;                    \
	}										\
}

namespace aka {
namespace gfx {

template <typename T> struct VulkanTypeTrait { static const VkObjectType debugType = VK_OBJECT_TYPE_UNKNOWN; };
template <> struct VulkanTypeTrait<VkImage> { static const VkObjectType debugType = VK_OBJECT_TYPE_IMAGE; };
template <> struct VulkanTypeTrait<VkDeviceMemory> { static const VkObjectType debugType = VK_OBJECT_TYPE_DEVICE_MEMORY; };
template <> struct VulkanTypeTrait<VkImageView> { static const VkObjectType debugType = VK_OBJECT_TYPE_IMAGE_VIEW; };
template <> struct VulkanTypeTrait<VkBuffer> { static const VkObjectType debugType = VK_OBJECT_TYPE_BUFFER; };
template <> struct VulkanTypeTrait<VkCommandPool> { static const VkObjectType debugType = VK_OBJECT_TYPE_COMMAND_POOL; };
template <> struct VulkanTypeTrait<VkRenderPass> { static const VkObjectType debugType = VK_OBJECT_TYPE_RENDER_PASS; };
template <> struct VulkanTypeTrait<VkSampler> { static const VkObjectType debugType = VK_OBJECT_TYPE_SAMPLER; };
template <> struct VulkanTypeTrait<VkShaderModule> { static const VkObjectType debugType = VK_OBJECT_TYPE_SHADER_MODULE; };
template <> struct VulkanTypeTrait<VkDescriptorSet> { static const VkObjectType debugType = VK_OBJECT_TYPE_DESCRIPTOR_SET; };
template <> struct VulkanTypeTrait<VkDescriptorSetLayout> { static const VkObjectType debugType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; };
template <> struct VulkanTypeTrait<VkDescriptorPool> { static const VkObjectType debugType = VK_OBJECT_TYPE_DESCRIPTOR_POOL; };
template <> struct VulkanTypeTrait<VkFence> { static const VkObjectType debugType = VK_OBJECT_TYPE_FENCE; };
template <> struct VulkanTypeTrait<VkSemaphore> { static const VkObjectType debugType = VK_OBJECT_TYPE_SEMAPHORE; };
template <> struct VulkanTypeTrait<VkQueue> { static const VkObjectType debugType = VK_OBJECT_TYPE_QUEUE; };

// Get a vulkan error string
const char* getVkErrorString(VkResult result);
// Get an object name
const char* getVkObjectName(VkObjectType objectType);
// Get allocation callback for given object type
VkAllocationCallbacks* getVkAllocator(VkObjectType objectType);
// Get allocation callback for given object type
template<typename T> VkAllocationCallbacks* getVkAllocator() { return getVkAllocator(VulkanTypeTrait<T>::debugType); }

};
};

#endif