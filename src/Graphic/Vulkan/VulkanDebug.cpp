#include "VulkanDebug.h"

#include <Aka/OS/Logger.h>
#include <Aka/Memory/Memory.h>
#include <Aka/Core/Container/HashMap.hpp>

#include <map>

#if defined(AKA_USE_VULKAN)
namespace aka {
namespace gfx {

const char* getVkErrorString(VkResult result)
{
	switch (result) 
	{
	case VK_SUCCESS: return "VK_SUCCESS";
	case VK_NOT_READY: return "VK_NOT_READY";
	case VK_TIMEOUT: return "VK_TIMEOUT";
	case VK_EVENT_SET: return "VK_EVENT_SET";
	case VK_EVENT_RESET: return "VK_EVENT_RESET";
	case VK_INCOMPLETE: return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
	case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
	case VK_ERROR_FRAGMENTATION_EXT: return "VK_ERROR_FRAGMENTATION_EXT";
	case VK_ERROR_NOT_PERMITTED_EXT: return "VK_ERROR_NOT_PERMITTED_EXT";
	case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT: return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
	case VK_RESULT_MAX_ENUM: return "VK_RESULT_MAX_ENUM";
	default: return "VK_ERROR_UNKNOWN";
	}
}

const char* getVkObjectName(VkObjectType objectType)
{
	switch (objectType)
	{
	default: return "VkUnknown";
	case VK_OBJECT_TYPE_INSTANCE: return "VkInstance";
	case VK_OBJECT_TYPE_PHYSICAL_DEVICE: return "VkPhysicalDevice";
	case VK_OBJECT_TYPE_DEVICE: return "VkDevice";
	case VK_OBJECT_TYPE_QUEUE: return "VkQueue";
	case VK_OBJECT_TYPE_SEMAPHORE: return "VkSemaphore";
	case VK_OBJECT_TYPE_COMMAND_BUFFER: return "VkCommandBuffer";
	case VK_OBJECT_TYPE_FENCE: return "VkFence";
	case VK_OBJECT_TYPE_DEVICE_MEMORY: return "VkDeviceMemory";
	case VK_OBJECT_TYPE_BUFFER: return "VkBuffer";
	case VK_OBJECT_TYPE_IMAGE: return "VkImage";
	case VK_OBJECT_TYPE_EVENT: return "VkEvent";
	case VK_OBJECT_TYPE_QUERY_POOL: return "VkQueryPool";
	case VK_OBJECT_TYPE_BUFFER_VIEW: return "VkBufferView";
	case VK_OBJECT_TYPE_IMAGE_VIEW: return "VkImageView";
	case VK_OBJECT_TYPE_SHADER_MODULE: return "VkShaderModule";
	case VK_OBJECT_TYPE_PIPELINE_CACHE: return "VkPipelineCache";
	case VK_OBJECT_TYPE_PIPELINE_LAYOUT: return "VkPipelineLayout";
	case VK_OBJECT_TYPE_RENDER_PASS: return "VkRenderPass";
	case VK_OBJECT_TYPE_PIPELINE: return "VkPipeline";
	case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: return "VkDescriptorSetLayout";
	case VK_OBJECT_TYPE_SAMPLER: return "VkSampler";
	case VK_OBJECT_TYPE_DESCRIPTOR_POOL: return "VkDescriptorPool";
	case VK_OBJECT_TYPE_DESCRIPTOR_SET: return "VkDescriptorSet";
	case VK_OBJECT_TYPE_FRAMEBUFFER: return "VkFramebuffer";
	case VK_OBJECT_TYPE_COMMAND_POOL: return "VkCommandPool";
	case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION: return "VkSamplerYcbcrConversion";
	case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE: return "VkDescriptorUpdateTemplate";
	case VK_OBJECT_TYPE_SURFACE_KHR: return "VkSurfaceKHR";
	case VK_OBJECT_TYPE_SWAPCHAIN_KHR: return "VkSwapchainKHR";
	case VK_OBJECT_TYPE_DISPLAY_KHR: return "VkDisplayKHR";
	case VK_OBJECT_TYPE_DISPLAY_MODE_KHR: return "VkDisplayModeKHR";
	case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT: return "VkDebugReportCallbackEXT";
	case VK_OBJECT_TYPE_CU_MODULE_NVX: return "VkCuModuleNVX";
	case VK_OBJECT_TYPE_CU_FUNCTION_NVX: return "VkCuFunctionNVX";
	case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT: return "VkDebugUtilsMessengerEXT";
	case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR: return "VkAccelerationStructureKHR";
	case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT: return "VkValidationCacheEXT";
	case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV: return "VkAccelerationStructureNV";
	}
}
#if defined(VK_DEBUG_ALLOCATION)
// Tag for memory tracking
struct VulkanHandle {
	byte_t byte;
};
static_assert(sizeof(VulkanHandle) == 1);
#endif

VkAllocationCallbacks* getVkAllocator()
{
#if defined(VK_DEBUG_ALLOCATION)
	static std::unique_ptr<VkAllocationCallbacks> allocator = nullptr;
	if (allocator == nullptr)
	{
		allocator = std::make_unique<VkAllocationCallbacks>();
		allocator->pfnAllocation = [](void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) -> void*
		{
			return mem::getAllocator(AllocatorMemoryType::Persistent, AllocatorCategory::Graphic).alignedAllocate<VulkanHandle>(size, alignment, AllocatorFlags::None);
		};
		allocator->pfnFree = [](void* pUserData, void* pMemory)
		{
			mem::getAllocator(AllocatorMemoryType::Persistent, AllocatorCategory::Graphic).alignedDeallocate(pMemory);
		};
		allocator->pfnInternalAllocation = nullptr; // Should do ?
		allocator->pfnInternalFree = nullptr; // Should do ?
		allocator->pfnReallocation = [](void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) -> void*
		{
			// No realloc, so free + alloc
			mem::getAllocator(AllocatorMemoryType::Persistent, AllocatorCategory::Graphic).alignedDeallocate(pOriginal);
			return mem::getAllocator(AllocatorMemoryType::Persistent, AllocatorCategory::Graphic).alignedAllocate<VulkanHandle>(size, alignment, AllocatorFlags::None);
		};
		allocator->pUserData = nullptr;
	}
	return allocator.get();
#else
	return nullptr; // Default allocator
#endif
}

};
};
#endif
