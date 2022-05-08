#pragma once

#if defined(AKA_USE_VULKAN)

#include "VulkanContext.h"

#define VK_DEBUG_ALLOCATION

namespace aka {
namespace gfx {

// Get an object name
const char* vkGetObjectName(VkObjectType objectType);
// Get allocation callback for given object type
VkAllocationCallbacks* vkGetAllocationCallbacks(VkObjectType objectType);

};
};

#endif