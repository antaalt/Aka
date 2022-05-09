#include "VulkanDebug.h"

#include <Aka/OS/Logger.h>

#include <map>

#if defined(AKA_USE_VULKAN)
namespace aka {
namespace gfx {

const char* vkGetErrorString(VkResult result)
{
	switch (result) {
	case VK_SUCCESS:
		return "VK_SUCCESS";
	case VK_NOT_READY:
		return "VK_NOT_READY";
	case VK_TIMEOUT:
		return "VK_TIMEOUT";
	case VK_EVENT_SET:
		return "VK_EVENT_SET";
	case VK_EVENT_RESET:
		return "VK_EVENT_RESET";
	case VK_INCOMPLETE:
		return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST:
		return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL:
		return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR:
		return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "VK_ERROR_VALIDATION_FAILED_EXT";
	case VK_ERROR_INVALID_SHADER_NV:
		return "VK_ERROR_INVALID_SHADER_NV";
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
	case VK_ERROR_FRAGMENTATION_EXT:
		return "VK_ERROR_FRAGMENTATION_EXT";
	case VK_ERROR_NOT_PERMITTED_EXT:
		return "VK_ERROR_NOT_PERMITTED_EXT";
	case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
		return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
	case VK_RESULT_MAX_ENUM:
		return "VK_RESULT_MAX_ENUM";
	default:
		return "VK_UNKNOWN";
	}
}

const char* vkGetObjectName(VkObjectType objectType)
{
	switch (objectType)
	{
	case VK_OBJECT_TYPE_UNKNOWN:
		return "VK_OBJECT_TYPE_UNKNOWN";
	case VK_OBJECT_TYPE_INSTANCE:
		return "VK_OBJECT_TYPE_INSTANCE";
	case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
		return "VK_OBJECT_TYPE_PHYSICAL_DEVICE";
	case VK_OBJECT_TYPE_DEVICE:
		return "VK_OBJECT_TYPE_DEVICE";
	case VK_OBJECT_TYPE_QUEUE:
		return "VK_OBJECT_TYPE_QUEUE";
	case VK_OBJECT_TYPE_SEMAPHORE:
		return "VK_OBJECT_TYPE_SEMAPHORE";
	case VK_OBJECT_TYPE_COMMAND_BUFFER:
		return "VK_OBJECT_TYPE_COMMAND_BUFFER";
	case VK_OBJECT_TYPE_FENCE:
		return "VK_OBJECT_TYPE_FENCE";
	case VK_OBJECT_TYPE_DEVICE_MEMORY:
		return "VK_OBJECT_TYPE_DEVICE_MEMORY";
	case VK_OBJECT_TYPE_BUFFER:
		return "VK_OBJECT_TYPE_BUFFER";
	case VK_OBJECT_TYPE_IMAGE:
		return "VK_OBJECT_TYPE_IMAGE";
	case VK_OBJECT_TYPE_EVENT:
		return "VK_OBJECT_TYPE_EVENT";
	case VK_OBJECT_TYPE_QUERY_POOL:
		return "VK_OBJECT_TYPE_QUERY_POOL";
	case VK_OBJECT_TYPE_BUFFER_VIEW:
		return "VK_OBJECT_TYPE_BUFFER_VIEW";
	case VK_OBJECT_TYPE_IMAGE_VIEW:
		return "VK_OBJECT_TYPE_IMAGE_VIEW";
	case VK_OBJECT_TYPE_SHADER_MODULE:
		return "VK_OBJECT_TYPE_SHADER_MODULE";
	case VK_OBJECT_TYPE_PIPELINE_CACHE:
		return "VK_OBJECT_TYPE_PIPELINE_CACHE";
	case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
		return "VK_OBJECT_TYPE_PIPELINE_LAYOUT";
	case VK_OBJECT_TYPE_RENDER_PASS:
		return "VK_OBJECT_TYPE_RENDER_PASS";
	case VK_OBJECT_TYPE_PIPELINE:
		return "VK_OBJECT_TYPE_PIPELINE";
	case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
		return "VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT";
	case VK_OBJECT_TYPE_SAMPLER:
		return "VK_OBJECT_TYPE_SAMPLER";
	case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
		return "VK_OBJECT_TYPE_DESCRIPTOR_POOL";
	case VK_OBJECT_TYPE_DESCRIPTOR_SET:
		return "VK_OBJECT_TYPE_DESCRIPTOR_SET";
	case VK_OBJECT_TYPE_FRAMEBUFFER:
		return "VK_OBJECT_TYPE_FRAMEBUFFER";
	case VK_OBJECT_TYPE_COMMAND_POOL:
		return "VK_OBJECT_TYPE_COMMAND_POOL";
	case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:
		return "VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION";
	case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:
		return "VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE";
	case VK_OBJECT_TYPE_SURFACE_KHR:
		return "VK_OBJECT_TYPE_SURFACE_KHR";
	case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
		return "VK_OBJECT_TYPE_SWAPCHAIN_KHR";
	case VK_OBJECT_TYPE_DISPLAY_KHR:
		return "VK_OBJECT_TYPE_DISPLAY_KHR";
	case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
		return "VK_OBJECT_TYPE_DISPLAY_MODE_KHR";
	case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
		return "VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT";
	case VK_OBJECT_TYPE_CU_MODULE_NVX:
		return "VK_OBJECT_TYPE_CU_MODULE_NVX";
	case VK_OBJECT_TYPE_CU_FUNCTION_NVX:
		return "VK_OBJECT_TYPE_CU_FUNCTION_NVX";
	case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
		return "VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT";
	case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR:
		return "VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR";
	case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:
		return "VK_OBJECT_TYPE_VALIDATION_CACHE_EXT";
	case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV:
		return "VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV";
	default:
		break;
	}
	return "unknown";
}

void* allocateCallback(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	void* ptr = malloc(size); // TODO align this
	memset(ptr, 0, size);
	Logger::debug("[vulkan] Allocating ", (const char*)pUserData, " (bytes: ", size, ", alignment:", alignment, ", scope:", allocationScope, ")");
	return ptr;
}
void freeCallback(void* pUserData, void* pMemory)
{
	::free(pMemory);
	Logger::debug("[vulkan] Freeing ", (const char*)pUserData);
}
void internalAllocateCallback(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
	Logger::debug("[vulkan] Internal allocating ", (const char*)pUserData, " (bytes: ", size, ", type:", allocationType, ", scope:", allocationScope, ")");
}
void internalFreeCallback(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
	Logger::debug("[vulkan] Internal freeing ", (const char*)pUserData, " (bytes: ", size, ", type:", allocationType, ", scope:", allocationScope, ")");
}
void* reallocationCallback(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	Logger::debug("[vulkan] Reallocating ", (const char*)pUserData, " (bytes: ", size, ", alignment:", alignment, ", scope:", allocationScope, ")");
	return realloc(pOriginal, size);
}

VkAllocationCallbacks* vkGetAllocationCallbacks(VkObjectType objectType)
{
#if defined(VK_DEBUG_ALLOCATION)
	static std::map<VkObjectType, std::unique_ptr<VkAllocationCallbacks>> callbacks;
	auto it = callbacks.find(objectType);
	if (it == callbacks.end())
	{
		std::unique_ptr<VkAllocationCallbacks> callback = std::make_unique<VkAllocationCallbacks>();
		callback->pfnAllocation = allocateCallback;
		callback->pfnFree = freeCallback;
		callback->pfnInternalAllocation = internalAllocateCallback;
		callback->pfnInternalFree = internalFreeCallback;
		callback->pfnReallocation = reallocationCallback;
		callback->pUserData = (void*)vkGetObjectName(objectType);
		VkAllocationCallbacks* ptr = callback.get();
		auto it = callbacks.insert(std::make_pair(objectType, std::move(callback)));
		return ptr;
	}
	return it->second.get();
#else
	return nullptr; // Default allocator
#endif
}

};
};
#endif
