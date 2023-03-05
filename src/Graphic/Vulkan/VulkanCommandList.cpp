#include "VulkanCommandList.h"

#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "VulkanGraphicDevice.h"

#if defined(AKA_USE_VULKAN)

namespace aka {
namespace gfx {

CommandList* VulkanGraphicDevice::acquireCommandList()
{
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.commandPool = m_context.commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer cmd = VK_NULL_HANDLE;
	VkResult result = vkAllocateCommandBuffers(m_context.device, &allocateInfo, &cmd);
	if (result != VK_SUCCESS || cmd == VK_NULL_HANDLE)
		return nullptr;
	// TODO store a pool of vulkan command buffer
	return new VulkanCommandList(m_context.device, cmd);
}

void VulkanGraphicDevice::release(CommandList* cmd)
{
	VulkanCommandList* vk_cmd = reinterpret_cast<VulkanCommandList*>(cmd);
	VK_CHECK_RESULT(vkQueueWaitIdle(m_context.graphicQueue.queue)); // TODO check correct queue
	vkFreeCommandBuffers(m_context.device, m_context.commandPool, 1, &vk_cmd->vk_command);
	// Release from pool
	delete cmd;
}

void VulkanGraphicDevice::submit(CommandList** commands, uint32_t count, QueueType queue)
{
	VulkanCommandList** vk_commands = reinterpret_cast<VulkanCommandList**>(commands);
	VkQueue vk_queue = getQueue(queue);

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = count;
	std::vector<VkCommandBuffer> cmds(count);
	for (uint32_t i = 0; i < count; i++)
		cmds[i] = vk_commands[i]->vk_command;
	submitInfo.pCommandBuffers = cmds.data();
	submitInfo.pWaitDstStageMask = waitStages;

	// Dirty temp hack
	// TODO move semaphore & fence to command list.
	// Cache them until we need them.
	// problem : who own them ?
	VkFence fence = m_swapchain.frames[m_swapchain.currentFrameIndex.value].presentFence;
	VkSemaphore signalSemaphore = m_swapchain.frames[m_swapchain.currentFrameIndex.value].presentSemaphore;
	VkSemaphore waitSemaphore = m_swapchain.frames[m_swapchain.currentFrameIndex.value].acquireSemaphore;
	VK_CHECK_RESULT(vkResetFences(m_context.device, 1, &fence));
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &signalSemaphore;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &waitSemaphore;
	// / Dirty temp hack

	VK_CHECK_RESULT(vkQueueSubmit(vk_queue, 1, &submitInfo, fence));
}

void VulkanGraphicDevice::wait(QueueType queue)
{
	VK_CHECK_RESULT(vkQueueWaitIdle(getQueue(queue)));
}

VkQueue VulkanGraphicDevice::getQueue(QueueType type)
{
	switch (type)
	{
	default:
		return VK_NULL_HANDLE;
	case QueueType::Graphic:
		return m_context.graphicQueue.queue;
		break;
	case QueueType::Compute:
		return m_context.graphicQueue.queue;
		break;
	case QueueType::Copy:
		return m_context.graphicQueue.queue;
		break;
	}
}

VulkanCommandList::VulkanCommandList(VkDevice device, VkCommandBuffer command) :
	vk_graphicPipeline(nullptr),
	vk_computePipeline(nullptr),
	vk_framebuffer(nullptr),
	vk_indices(nullptr),
	vk_vertices(nullptr),
	vk_device(device),
	vk_command(command),
	m_recording(false)
{
}

void VulkanCommandList::begin()
{
	AKA_ASSERT(!m_recording, "Trying to begin a command buffer that is already recording");

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;// SIMULTANEOUS_USE_BIT;

	VK_CHECK_RESULT(vkBeginCommandBuffer(vk_command, &beginInfo));
	m_recording = true;
}

void VulkanCommandList::end() 
{
	AKA_ASSERT(m_recording, "Trying to end a command buffer that is not recording");
	VK_CHECK_RESULT(vkEndCommandBuffer(vk_command));
	m_recording = false;
}

void VulkanCommandList::reset()
{
	vkResetCommandBuffer(vk_command, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void VulkanCommandList::beginRenderPass(FramebufferHandle framebuffer, const ClearState& clear)
{
	VulkanFramebuffer* vk_framebuffer = get<VulkanFramebuffer>(framebuffer);
	AKA_ASSERT(m_recording, "Trying to record something but not recording");

	// TODO clear mask should be set at renderpass level.
	std::vector<VkClearValue> clearValues(vk_framebuffer->framebuffer.count);
	for (VkClearValue& vk_clear : clearValues)
	{
		vk_clear.depthStencil = VkClearDepthStencilValue{ clear.depth, clear.stencil };
		memcpy(vk_clear.color.float32, clear.color, sizeof(float) * 4);
	}
	if (vk_framebuffer->hasDepthStencil())
	{
		VkClearValue depthClear{};
		depthClear.depthStencil = VkClearDepthStencilValue{ clear.depth, clear.stencil };
		clearValues.push_back(depthClear);
	}

	// Transition to attachment optimal if shader resource
	if (vk_framebuffer->hasDepthStencil())
	{
		AKA_ASSERT(has(vk_framebuffer->depth.texture.data->flags, TextureFlag::RenderTarget), "Invalid attachment");
		if (has(vk_framebuffer->depth.texture.data->flags, TextureFlag::ShaderResource))
		{
			VulkanTexture* vk_texture = get<VulkanTexture>(vk_framebuffer->depth.texture);
			if (vk_texture->vk_layout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				vk_texture->transitionImageLayout(
					vk_command,
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
					VkImageSubresourceRange{ 
						VulkanTexture::getAspectFlag(vk_texture->format),
						0, vk_texture->levels,
						0, vk_texture->layers
					}
				);
			}
		}
	}
	for ( uint32_t i = 0; i < vk_framebuffer->framebuffer.count; i++)
	{
		const Attachment& att = vk_framebuffer->colors[i];
		VulkanTexture* vk_texture = get<VulkanTexture>(att.texture);
		AKA_ASSERT(has(vk_texture->flags, TextureFlag::RenderTarget), "Invalid attachment");
		if (has(vk_texture->flags, TextureFlag::ShaderResource))
		{
			// TODO this check is not in sync with async command buffer and will work only when using a single cmd buffer
			if (vk_texture->vk_layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			{
				vk_texture->transitionImageLayout(
					vk_command,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VkImageSubresourceRange{
						VulkanTexture::getAspectFlag(vk_texture->format),
						0, vk_texture->levels,
						0, vk_texture->layers
					}
				);
			}
		}
	}

	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = vk_framebuffer->vk_renderpass;
	beginInfo.framebuffer = vk_framebuffer->vk_framebuffer;
	beginInfo.renderArea.offset = VkOffset2D{ 0, 0 };
	beginInfo.renderArea.extent = VkExtent2D{ vk_framebuffer->width, vk_framebuffer->height };
	beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	beginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(vk_command, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
	this->vk_framebuffer = vk_framebuffer;
}
void VulkanCommandList::endRenderPass() 
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	vkCmdEndRenderPass(vk_command);

	// Transition to shader resource optimal
	if (vk_framebuffer->hasDepthStencil())
	{
		AKA_ASSERT(has(vk_framebuffer->depth.texture.data->flags, TextureFlag::RenderTarget), "Invalid attachment");
		if (has(vk_framebuffer->depth.texture.data->flags, TextureFlag::ShaderResource))
		{
			VulkanTexture* vk_texture = get<VulkanTexture>(vk_framebuffer->depth.texture);
			//if (vk_texture->vk_layout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
			{
				vk_texture->insertMemoryBarrier(
					vk_command,
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
					VkImageSubresourceRange{
						VulkanTexture::getAspectFlag(vk_texture->format),
						0, vk_texture->levels,
						0, vk_texture->layers
					},
					VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT
				);
			}
		}
	}
	for (uint32_t i = 0; i < vk_framebuffer->framebuffer.count; i++)
	{
		const Attachment& att = vk_framebuffer->colors[i];
		VulkanTexture* vk_texture = get<VulkanTexture>(att.texture);
		AKA_ASSERT(has(vk_texture->flags, TextureFlag::RenderTarget), "Invalid attachment");
		if (has(vk_texture->flags, TextureFlag::ShaderResource))
		{
			AKA_ASSERT(!vk_framebuffer->isSwapchain, "Swapchain should not be a shader resource.");
			// TODO this check is not in sync with async command buffer and will work only when using a single cmd buffer
			//if (vk_texture->vk_layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				vk_texture->insertMemoryBarrier(
					vk_command,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // vk_framebuffer->isSwapchain ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
					VkImageSubresourceRange{
						VulkanTexture::getAspectFlag(vk_texture->format),
						0, vk_texture->levels,
						0, vk_texture->layers
					},
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT
				);
			}
		}
	}
	
	this->vk_framebuffer = nullptr;
}

void VulkanCommandList::bindPipeline(GraphicPipelineHandle pipeline)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	VulkanGraphicPipeline* vk_pipeline = get<VulkanGraphicPipeline>(pipeline);
	VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	vkCmdBindPipeline(vk_command, bindPoint, vk_pipeline->vk_pipeline);
	
	this->vk_graphicPipeline = vk_pipeline;
	this->vk_computePipeline = nullptr;
}

void VulkanCommandList::bindPipeline(ComputePipelineHandle pipeline)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	VulkanComputePipeline* vk_pipeline = get<VulkanComputePipeline>(pipeline);
	VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
	vkCmdBindPipeline(vk_command, bindPoint, vk_pipeline->vk_pipeline);

	this->vk_graphicPipeline = nullptr;
	this->vk_computePipeline = vk_pipeline;
}

void VulkanCommandList::bindDescriptorSet(uint32_t index, DescriptorSetHandle set)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	AKA_ASSERT(vk_graphicPipeline != nullptr || vk_computePipeline != nullptr, "Invalid pipeline");

	bool graphicPipeline = this->vk_graphicPipeline != nullptr;
	bool computePipeline = this->vk_computePipeline != nullptr;
	VkPipelineBindPoint bindPoint = graphicPipeline ? VK_PIPELINE_BIND_POINT_GRAPHICS : computePipeline ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_MAX_ENUM;
	if (set.data->bindings.count > 0)
	{
		VkPipelineLayout vk_layout = VK_NULL_HANDLE;
		if (vk_graphicPipeline)
			vk_layout = vk_graphicPipeline->vk_pipelineLayout;
		else if (vk_computePipeline)
			vk_layout = vk_computePipeline->vk_pipelineLayout;
		const VulkanDescriptorSet* vk_material = reinterpret_cast<const VulkanDescriptorSet*>(set.data);
		vkCmdBindDescriptorSets(vk_command, bindPoint, vk_layout, index, 1, &vk_material->vk_descriptorSet, 0, nullptr);
		this->vk_sets[index] = vk_material;
	}
}
void VulkanCommandList::bindDescriptorSets(const DescriptorSetHandle* sets, uint32_t count)
{
	if (count < 1)
		return;
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	AKA_ASSERT(vk_graphicPipeline != nullptr || vk_computePipeline != nullptr, "Invalid pipeline");

	VkPipelineBindPoint vk_bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // TODO compute & RT
	VkDescriptorSet vk_sets[8]{}; // Max bindings
	VkPipelineLayout vk_layout = VK_NULL_HANDLE;
	if (vk_graphicPipeline)
		vk_layout = vk_graphicPipeline->vk_pipelineLayout;
	else if (vk_computePipeline)
		vk_layout = vk_computePipeline->vk_pipelineLayout;
	for (uint32_t i = 0; i < count; i++)
	{
		if (sets[i].data->bindings.count > 0)
		{
			const VulkanDescriptorSet* vk_set = reinterpret_cast<const VulkanDescriptorSet*>(sets[i].data);
			vk_sets[i] = vk_set->vk_descriptorSet;
			this->vk_sets[i] = vk_set;
		}
		else
		{
			vk_sets[i] = VK_NULL_HANDLE;
		}
	}
	vkCmdBindDescriptorSets(vk_command, vk_bindPoint, vk_layout, 0, count, vk_sets, 0, nullptr);
}
void VulkanCommandList::bindVertexBuffer(const BufferHandle* buffers, uint32_t binding, uint32_t bindingCount, const uint32_t* offsets)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	VkBuffer vk_buffers[VertexMaxAttributeCount]{};
	VkDeviceSize vk_offsets[VertexMaxAttributeCount]{};
	for (size_t i = 0; i < bindingCount; i++)
	{
		VulkanBuffer* vk_buffer = get<VulkanBuffer>(buffers[i]);
		vk_buffers[i] = vk_buffer->vk_buffer;
		vk_offsets[i] = offsets[i];
	}
	vkCmdBindVertexBuffers(vk_command, binding, bindingCount, vk_buffers, vk_offsets);
	//this->vk_vertices = vk_buffer;
}
void VulkanCommandList::bindIndexBuffer(BufferHandle buffer, IndexFormat format, uint32_t offset)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	VulkanBuffer* vk_buffer = get<VulkanBuffer>(buffer);
	VkBuffer buffers = vk_buffer->vk_buffer;
	VkIndexType indexType = VulkanContext::tovk(format);
	vkCmdBindIndexBuffer(vk_command, buffers, offset, indexType);
	this->vk_indices = vk_buffer;
}

void VulkanCommandList::clear(ClearMask mask, const float* color, float depth, uint32_t stencil) 
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	AKA_ASSERT(vk_framebuffer != nullptr, "Need an active render pass.");
	VkImageAspectFlags flags = 0;
	if (has(mask, ClearMask::Color))
		flags |= VK_IMAGE_ASPECT_COLOR_BIT;
	if (has(mask, ClearMask::Depth))
		flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
	if (has(mask, ClearMask::Stencil))
		flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
	//vk_pipeline->renderPass.
	std::vector<VkClearAttachment> attachments;
	for (uint32_t i = 0; i < this->vk_framebuffer->framebuffer.count; i++)
	{
		VkClearAttachment att{};
		att.aspectMask = flags;
		att.clearValue.color = VkClearColorValue{ color[0], color[1], color[2], color[3] };
		att.colorAttachment = i;
		attachments.push_back(att);

	}
	if (this->vk_framebuffer->depth.texture != gfx::TextureHandle::null)
	{
		VkClearAttachment att{};
		att.aspectMask = flags;
		att.clearValue.depthStencil = VkClearDepthStencilValue{ depth, stencil };
		att.colorAttachment = this->vk_framebuffer->framebuffer.count; // depth is last
		attachments.push_back(att);
	}

	VkClearRect clearRect{};
	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;
	clearRect.rect.extent = VkExtent2D{ vk_framebuffer->width, vk_framebuffer->height };
	clearRect.rect.offset = VkOffset2D{ 0, 0 };
	vkCmdClearAttachments(vk_command, (uint32_t)attachments.size(), attachments.data(), 1, &clearRect);
}

void VulkanCommandList::draw(uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceCount) 
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	uint32_t instanceOffset = 0;
	vkCmdDraw(vk_command, vertexCount, instanceCount, vertexOffset, instanceOffset);
}
void VulkanCommandList::drawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceCount) 
{
	// TODO check binded render target are not also binded as input.
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	uint32_t instanceOffset = 0;
	vkCmdDrawIndexed(vk_command, indexCount, instanceCount, indexOffset, vertexOffset, instanceOffset);
}
void VulkanCommandList::dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ) 
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	vkCmdDispatch(vk_command, groupX, groupY, groupZ);
}

void VulkanCommandList::copy(TextureHandle src, TextureHandle dst)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	VulkanTexture* vk_src = get<VulkanTexture>(src);
	VulkanTexture* vk_dst = get<VulkanTexture>(dst);

	vk_dst->copyFrom(vk_command, vk_src);
}

void VulkanCommandList::blit(TextureHandle src, TextureHandle dst, const BlitRegion& srcRegion, const BlitRegion& dstRegion, Filter filter)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	VulkanTexture* vk_src = get<VulkanTexture>(src);
	VulkanTexture* vk_dst = get<VulkanTexture>(dst);

	vk_dst->blitFrom(vk_command, vk_src, srcRegion, dstRegion, filter);
}

VkCommandBuffer VulkanCommandList::createSingleTime(VkDevice device, VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	return commandBuffer;
}

void VulkanCommandList::endSingleTime(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue graphicsQueue)
{
	VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VK_CHECK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK_RESULT(vkQueueWaitIdle(graphicsQueue));

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

};
};

#endif