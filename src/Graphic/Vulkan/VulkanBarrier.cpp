#if defined(AKA_USE_VULKAN)

#include "VulkanTexture.h"
#include "VulkanGraphicDevice.h"
#include "VulkanBarrier.h"

namespace aka {
namespace gfx {


// https://themaister.net/blog/2019/08/14/yet-another-blog-explaining-vulkan-synchronization/
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineStageFlagBits.html
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAccessFlagBits.html
// This is the stages in pipeline
//	   Graphic Queue						Compute Queue			Copy Queue
//	   _________________________________________________________________________
//
//     TOP_OF_PIPE_BIT						TOP_OF_PIPE_BIT 		TOP_OF_PIPE_BIT
//     DRAW_INDIRECT_BIT					COMPUTE_SHADER_BIT		TRANSFER_BIT	
//     VERTEX_INPUT_BIT						BOTTOM_OF_PIPE_BIT		BOTTOM_OF_PIPE_BIT
//     VERTEX_SHADER_BIT
//     TESSELLATION_CONTROL_SHADER_BIT
//     TESSELLATION_EVALUATION_SHADER_BIT
//     GEOMETRY_SHADER_BIT
//     FRAGMENT_SHADER_BIT
//     EARLY_FRAGMENT_TESTS_BIT
//     LATE_FRAGMENT_TESTS_BIT
//     COLOR_ATTACHMENT_OUTPUT_BIT
//     TRANSFER_BIT
//     COMPUTE_SHADER_BIT 
//     BOTTOM_OF_PIPE_BIT
// 
// Should handle ShaderType as input for exact pipeline aswell.

// TODO: Handle resource should depend on custom enum PipelineStage (GraphicPipelineStage, ComputePipelineStage ?)
VkPipelineStageFlags getPipelineStage(QueueType queueType, bool asyncQueue)
{
	// Only if async queues.
	if (asyncQueue) 
	{
		switch (queueType)
		{
		case QueueType::Graphic:
			return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
				| VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
				| VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
#if 0 // Tesselation & mesh support...
				| VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
				| VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
				| VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT
				| VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT
#endif
			;
		case QueueType::Compute:
			return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		case QueueType::Copy:
			return VK_PIPELINE_STAGE_TRANSFER_BIT;
		default: AKA_UNREACHABLE;
			return VK_PIPELINE_STAGE_NONE;
		};
	}
	else 
	{
		return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
			| VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
			| VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
#if 0 // Tesselation & mesh support...
			| VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
			| VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
			| VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT
			| VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT
#endif
		;
	}
	
};
static const bool s_asyncQueues = true;
std::pair<VulkanPipelineStageAccess, VulkanPipelineStageAccess> getTexturePipelineStageAccess(QueueType _queueType, ResourceAccessType _old, ResourceAccessType _new, TextureFormat _format)
{
	bool depth = Texture::hasDepth(_format);
	bool stencil = Texture::hasStencil(_format);
	auto getOldPipelineStageFlags = [&](ResourceAccessType _type) -> VulkanPipelineStageAccess {
		switch (_type)
		{
		default:
			AKA_ASSERT(false, "Invalid access type");
			[[fallthrough]];
		case ResourceAccessType::Undefined:
			return { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_NONE };
		case ResourceAccessType::Resource: // Read only
			return { getPipelineStage(_queueType, s_asyncQueues), (uint32_t)((s_asyncQueues && _queueType == QueueType::Copy) ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_SHADER_READ_BIT) }; // Only for shader read
			//return VK_ACCESS_MEMORY_READ_BIT; // Most general purpose
			//return VK_ACCESS_HOST_READ_BIT; // access on host (VK_PIPELINE_STAGE_HOST_BIT)
		case ResourceAccessType::Attachment:
			if (_queueType == QueueType::Graphic || !s_asyncQueues) {
				// TODO: VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
				if (depth || stencil) return { VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};
				else return { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};
			} else {
				return { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_NONE };
			}
		case ResourceAccessType::Storage:
			return { getPipelineStage(_queueType, s_asyncQueues), (uint32_t)((s_asyncQueues && _queueType == QueueType::Copy) ? VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT : VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT) };
		case ResourceAccessType::CopySRC:
			return { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT }; // VK_ACCESS_HOST_READ_BIT
		case ResourceAccessType::CopyDST:
			return { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT };
		case ResourceAccessType::Present:
			return { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT };
		}
	};
	auto getNewPipelineStageFlags = [&](ResourceAccessType _type) -> VulkanPipelineStageAccess { 
		return getOldPipelineStageFlags(_type);
	};

	return std::make_pair(getOldPipelineStageFlags(_old), getNewPipelineStageFlags(_new));
}
std::pair<VulkanPipelineStageAccess, VulkanPipelineStageAccess> getBufferPipelineStageAccess(QueueType _queueType, ResourceAccessType _old, ResourceAccessType _new)
{
	auto getOldPipelineStageFlags = [&](ResourceAccessType _type) -> VulkanPipelineStageAccess {
		switch (_type)
		{
		default:
			AKA_ASSERT(false, "Invalid access type");
			[[fallthrough]];
		case ResourceAccessType::Undefined:
			return { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_NONE };
		case ResourceAccessType::Resource: // Read only
			return { getPipelineStage(_queueType, s_asyncQueues), (uint32_t)((s_asyncQueues && _queueType == QueueType::Copy) ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_SHADER_READ_BIT) }; // Only for shader read
			//return VK_ACCESS_MEMORY_READ_BIT; // Most general purpose
			//return VK_ACCESS_HOST_READ_BIT; // access on host (VK_PIPELINE_STAGE_HOST_BIT)
		case ResourceAccessType::Attachment:
			AKA_ASSERT(false, "You cannot transition a buffer to attachment...");
			return { VK_PIPELINE_STAGE_NONE, VK_ACCESS_NONE };
		case ResourceAccessType::Storage:
			return { getPipelineStage(_queueType, s_asyncQueues), (uint32_t)((s_asyncQueues && _queueType == QueueType::Copy) ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT) };
		case ResourceAccessType::CopySRC:
			return { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT }; // VK_ACCESS_HOST_READ_BIT
		case ResourceAccessType::CopyDST:
			return { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT };
		case ResourceAccessType::Present:
			AKA_ASSERT(false, "You cannot transition a buffer to present...");
			return { VK_PIPELINE_STAGE_NONE, VK_ACCESS_NONE };
		}
	};
	auto getNewPipelineStageFlags = [&](ResourceAccessType _type) -> VulkanPipelineStageAccess { 
		return getOldPipelineStageFlags(_type);
	};

	return std::make_pair(getOldPipelineStageFlags(_old), getNewPipelineStageFlags(_new));
}

};
};

#endif