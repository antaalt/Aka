#pragma once
#include <Aka/Graphic/GraphicDevice.h>

#if defined(AKA_USE_VULKAN)

#include "VulkanContext.h"

namespace aka {
namespace gfx {

struct VulkanPipelineStageAccess {
	VkPipelineStageFlags stage;
	VkAccessFlags access;
};
std::pair<VulkanPipelineStageAccess, VulkanPipelineStageAccess> getTexturePipelineStageAccess(QueueType _queueType, ResourceAccessType _old, ResourceAccessType _new, TextureFormat _format);
std::pair<VulkanPipelineStageAccess, VulkanPipelineStageAccess> getBufferPipelineStageAccess(QueueType _queueType, ResourceAccessType _old, ResourceAccessType _new);

} // namespace gfx
} // namespace aka

#endif