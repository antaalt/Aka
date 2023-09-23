#include "VulkanCommandList.h"

#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "VulkanGraphicDevice.h"

#if defined(AKA_USE_VULKAN)

namespace aka {
namespace gfx {

CommandList* VulkanGraphicDevice::acquireCommandList(QueueType queue)
{
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.commandPool = m_context.commandPool[EnumToIndex(queue)];
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer cmd = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_context.device, &allocateInfo, &cmd));

	return m_commandListPool.acquire(this, cmd, queue, true);
}

void VulkanGraphicDevice::release(CommandList* cmd)
{
	VulkanCommandList* vk_cmd = reinterpret_cast<VulkanCommandList*>(cmd);
	vkFreeCommandBuffers(m_context.device, getVkCommandPool(vk_cmd->m_queue), 1, &vk_cmd->vk_command);

	m_commandListPool.release();
}

void VulkanGraphicDevice::submit(CommandList* command, FenceHandle handle, FenceValue waitValue, FenceValue signalValue)
{
	VulkanCommandList* vk_command = reinterpret_cast<VulkanCommandList*>(command);
	VulkanFence* vk_fence = getVk<VulkanFence>(handle);

	VkQueue vk_queue = getVkQueue(vk_command->getQueueType());

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

	VkTimelineSemaphoreSubmitInfo timelineInfo;
	timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	timelineInfo.pNext = NULL;
	timelineInfo.waitSemaphoreValueCount = 1;
	timelineInfo.pWaitSemaphoreValues = &waitValue;
	timelineInfo.signalSemaphoreValueCount = 1;
	timelineInfo.pSignalSemaphoreValues = &signalValue;

	uint32_t semaphoreCount = 0;
	VkSemaphore waitSemaphore[1] = { VK_NULL_HANDLE };
	VkSemaphore signalSemaphore[1] = { VK_NULL_HANDLE };
	if (handle != FenceHandle::null)
	{
		semaphoreCount = 1;
		waitSemaphore[0] = vk_fence->vk_sempahore;
		signalSemaphore[0] = vk_fence->vk_sempahore; // TODO handle invalid values...
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vk_command->vk_command;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = semaphoreCount;
	submitInfo.pSignalSemaphores = signalSemaphore;
	submitInfo.waitSemaphoreCount = semaphoreCount;
	submitInfo.pWaitSemaphores = waitSemaphore;

	VK_CHECK_RESULT(vkQueueSubmit(vk_queue, 1, &submitInfo, VK_NULL_HANDLE));
}

void VulkanGraphicDevice::wait(QueueType queue)
{
	VK_CHECK_RESULT(vkQueueWaitIdle(getVkQueue(queue)));
}

CommandList* VulkanGraphicDevice::getCopyCommandList(Frame* frame)
{
	VulkanFrame* vk_frame = reinterpret_cast<VulkanFrame*>(frame);
	return &vk_frame->commandLists[EnumToIndex(QueueType::Copy)];
}

CommandList* VulkanGraphicDevice::getGraphicCommandList(Frame* frame)
{
	VulkanFrame* vk_frame = reinterpret_cast<VulkanFrame*>(frame);
	return &vk_frame->commandLists[EnumToIndex(QueueType::Graphic)];
}

CommandList* VulkanGraphicDevice::getComputeCommandList(Frame* frame)
{
	VulkanFrame* vk_frame = reinterpret_cast<VulkanFrame*>(frame);
	return &vk_frame->commandLists[EnumToIndex(QueueType::Compute)];
}

VulkanCommandList::VulkanCommandList() :
	VulkanCommandList(nullptr, VK_NULL_HANDLE, QueueType::Unknown, false)
{
}

VulkanCommandList::VulkanCommandList(VulkanGraphicDevice* device, VkCommandBuffer command, QueueType queue, bool oneTimeSubmit) :
	vk_graphicPipeline(nullptr),
	vk_computePipeline(nullptr),
	vk_framebuffer(nullptr),
	vk_indices(nullptr),
	vk_sets{},
	vk_vertices(nullptr),
	device(device),
	vk_command(command),
	m_recording(false),
	m_oneTimeSubmit(oneTimeSubmit),
	m_queue(queue)
{
}

void VulkanCommandList::begin()
{
	AKA_ASSERT(!m_recording, "Trying to begin a command buffer that is already recording");

	VK_CHECK_RESULT(vkResetCommandBuffer(vk_command, 0)); //VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = m_oneTimeSubmit ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VK_CHECK_RESULT(vkBeginCommandBuffer(vk_command, &beginInfo));
	m_recording = true;
}

void VulkanCommandList::end() 
{
	AKA_ASSERT(m_recording, "Trying to end a command buffer that is not recording");
	VK_CHECK_RESULT(vkEndCommandBuffer(vk_command));
	// Reset data
	vk_graphicPipeline = nullptr;
	vk_computePipeline = nullptr;
	for (uint32_t i = 0; i < ShaderMaxSetCount; i++)
		vk_sets[i] = nullptr;
	vk_framebuffer = nullptr;
	vk_indices = nullptr;
	vk_vertices = nullptr;
	// Stop recording
	m_recording = false;
}

void VulkanCommandList::reset()
{
	vkResetCommandBuffer(vk_command, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void VulkanCommandList::beginRenderPass(RenderPassHandle renderPass, FramebufferHandle framebuffer, const ClearState& clear)
{
	VulkanFramebuffer* vk_framebuffer = device->getVk<VulkanFramebuffer>(framebuffer);
	VulkanRenderPass* vk_renderPass = device->getVk<VulkanRenderPass>(renderPass);
	AKA_ASSERT(m_recording, "Trying to record something but not recording");

	std::vector<VkClearValue> clearValues(vk_framebuffer->count);
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

	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = vk_renderPass->vk_renderpass;
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

	this->vk_framebuffer = nullptr;
}

void VulkanCommandList::bindPipeline(GraphicPipelineHandle pipeline)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	VulkanGraphicPipeline* vk_pipeline = device->getVk<VulkanGraphicPipeline>(pipeline);
	VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	vkCmdBindPipeline(vk_command, bindPoint, vk_pipeline->vk_pipeline);
	
	this->vk_graphicPipeline = vk_pipeline;
	this->vk_computePipeline = nullptr;
}

void VulkanCommandList::bindPipeline(ComputePipelineHandle pipeline)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	VulkanComputePipeline* vk_pipeline = device->getVk<VulkanComputePipeline>(pipeline);
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
	const DescriptorSet* descriptorSet = device->get(set);
	if (descriptorSet->bindings.count > 0)
	{
		VkPipelineLayout vk_pipelineLayout = VK_NULL_HANDLE;
		if (vk_graphicPipeline)
			vk_pipelineLayout = vk_graphicPipeline->vk_pipelineLayout;
		else if (vk_computePipeline)
			vk_pipelineLayout = vk_computePipeline->vk_pipelineLayout;
		const VulkanDescriptorSet* vk_descriptor = reinterpret_cast<const VulkanDescriptorSet*>(descriptorSet);
		vkCmdBindDescriptorSets(vk_command, bindPoint, vk_pipelineLayout, index, 1, &vk_descriptor->vk_descriptorSet, 0, nullptr);
		this->vk_sets[index] = vk_descriptor;
	}
}
void VulkanCommandList::bindDescriptorSets(const DescriptorSetHandle* sets, uint32_t count)
{
	if (count < 1)
		return;
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	AKA_ASSERT(vk_graphicPipeline != nullptr || vk_computePipeline != nullptr, "Invalid pipeline");

	bool graphicPipeline = this->vk_graphicPipeline != nullptr;
	bool computePipeline = this->vk_computePipeline != nullptr;
	VkPipelineBindPoint vk_bindPoint = graphicPipeline ? VK_PIPELINE_BIND_POINT_GRAPHICS : computePipeline ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_MAX_ENUM;
	VkDescriptorSet vk_sets[ShaderMaxSetCount]{};
	VkPipelineLayout vk_pipelineLayout = VK_NULL_HANDLE;
	if (vk_graphicPipeline)
		vk_pipelineLayout = vk_graphicPipeline->vk_pipelineLayout;
	else if (vk_computePipeline)
		vk_pipelineLayout = vk_computePipeline->vk_pipelineLayout;
	for (uint32_t i = 0; i < count; i++)
	{
		const DescriptorSet* descriptorSet = device->get(sets[i]);
		if (descriptorSet->bindings.count > 0)
		{
			const VulkanDescriptorSet* vk_set = reinterpret_cast<const VulkanDescriptorSet*>(descriptorSet);
			vk_sets[i] = vk_set->vk_descriptorSet;
			this->vk_sets[i] = vk_set;
		}
		else
		{
			vk_sets[i] = VK_NULL_HANDLE;
		}
	}
	vkCmdBindDescriptorSets(vk_command, vk_bindPoint, vk_pipelineLayout, 0, count, vk_sets, 0, nullptr);
}
void VulkanCommandList::transition(TextureHandle texture, ResourceAccessType src, ResourceAccessType dst)
{
	VulkanTexture* vk_texture = device->getVk<VulkanTexture>(texture);
	VulkanTexture::transitionImageLayout(
		vk_command,
		vk_texture->vk_image,
		src,
		dst, 
		vk_texture->format
	);
}
void VulkanCommandList::transition(BufferHandle buffer, ResourceAccessType src, ResourceAccessType dst)
{
	VulkanBuffer* vk_buffer = device->getVk<VulkanBuffer>(buffer);
	VulkanBuffer::transitionBuffer(
		vk_command,
		vk_buffer->vk_buffer,
		vk_buffer->size,
		0,
		src,
		dst
	);
}
void VulkanCommandList::bindVertexBuffer(uint32_t binding, const BufferHandle buffers, uint32_t offset)
{
	bindVertexBuffers(binding, 1, &buffers, &offset);
}
void VulkanCommandList::bindVertexBuffers(uint32_t binding, uint32_t bindingCount, const BufferHandle* buffers, const uint32_t* offsets)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	VkBuffer vk_buffers[VertexMaxAttributeCount]{};
	VkDeviceSize vk_offsets[VertexMaxAttributeCount]{};
	for (size_t i = 0; i < bindingCount; i++)
	{
		VulkanBuffer* vk_buffer = device->getVk<VulkanBuffer>(buffers[i]);
		vk_buffers[i] = vk_buffer->vk_buffer;
		vk_offsets[i] = offsets ? offsets[i] : 0;
	}
	vkCmdBindVertexBuffers(vk_command, binding, bindingCount, vk_buffers, vk_offsets);
	//this->vk_vertices = vk_buffer;
}
void VulkanCommandList::bindIndexBuffer(BufferHandle buffer, IndexFormat format, uint32_t offset)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	VulkanBuffer* vk_buffer = device->getVk<VulkanBuffer>(buffer);
	VkBuffer buffers = vk_buffer->vk_buffer;
	VkIndexType indexType = VulkanContext::tovk(format);
	vkCmdBindIndexBuffer(vk_command, buffers, offset, indexType);
	this->vk_indices = vk_buffer;
}

void VulkanCommandList::clear(ClearMask mask, const float* color, float depth, uint32_t stencil) 
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	AKA_ASSERT(vk_framebuffer != nullptr, "Need an active render pass.");

	std::vector<VkClearAttachment> attachments;
	if (!isNull(mask & ClearMask::Color))
	{
		for (uint32_t i = 0; i < this->vk_framebuffer->count; i++)
		{
			const Texture* tex = device->get(vk_framebuffer->colors[i].texture);
			AKA_ASSERT(Texture::isColor(tex->format), "Texture is not color.");
			VkClearAttachment att{};
			att.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			att.clearValue.color = VkClearColorValue{ color[0], color[1], color[2], color[3] };
			att.colorAttachment = i;
			attachments.push_back(att);
		}
	}
	if (this->vk_framebuffer->depth.texture != gfx::TextureHandle::null)
	{
		const Texture* tex = device->get(vk_framebuffer->depth.texture);
		VkImageAspectFlags aspect = 0;
		if (Texture::hasDepth(tex->format))
			aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
		if (Texture::hasStencil(tex->format))
			aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
		AKA_ASSERT(aspect != 0, "Invalid depth image.");
		VkClearAttachment att{};
		att.aspectMask = aspect;
		att.clearValue.depthStencil = VkClearDepthStencilValue{ depth, stencil };
		att.colorAttachment = this->vk_framebuffer->count; // depth is last
		attachments.push_back(att);
	}

	VkClearRect clearRect{};
	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;
	clearRect.rect.extent = VkExtent2D{ vk_framebuffer->width, vk_framebuffer->height };
	clearRect.rect.offset = VkOffset2D{ 0, 0 };
	vkCmdClearAttachments(vk_command, (uint32_t)attachments.size(), attachments.data(), 1, &clearRect);
}

void VulkanCommandList::push(uint32_t offset, uint32_t range, const void* data, ShaderMask mask)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	VkPipelineLayout layout = VK_NULL_HANDLE;
	if (this->vk_graphicPipeline)
	{
		layout = this->vk_graphicPipeline->vk_pipelineLayout;
	}
	else if (this->vk_computePipeline)
	{
		layout = this->vk_computePipeline->vk_pipelineLayout;
	}
	else
	{
		AKA_ASSERT(false, "Invalid or no pipeline bound.");
	}

	vkCmdPushConstants(vk_command, layout, VulkanContext::tovk(mask), offset, range, data);
}

void VulkanCommandList::draw(uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceCount) 
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	uint32_t instanceOffset = 0;
	vkCmdDraw(vk_command, vertexCount, instanceCount, vertexOffset, instanceOffset);
}
void VulkanCommandList::drawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceCount) 
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	uint32_t instanceOffset = 0;
	vkCmdDrawIndexed(vk_command, indexCount, instanceCount, indexOffset, vertexOffset, instanceOffset);
}
void VulkanCommandList::dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ) 
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	vkCmdDispatch(vk_command, groupX, groupY, groupZ);
}

void VulkanCommandList::copy(BufferHandle src, BufferHandle dst)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	VulkanBuffer* vk_src = device->getVk<VulkanBuffer>(src);
	VulkanBuffer* vk_dst = device->getVk<VulkanBuffer>(dst);

	vk_dst->copyFrom(vk_command, vk_src);
}

void VulkanCommandList::copy(TextureHandle src, TextureHandle dst)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	VulkanTexture* vk_src = device->getVk<VulkanTexture>(src);
	VulkanTexture* vk_dst = device->getVk<VulkanTexture>(dst);

	vk_dst->copyFrom(vk_command, vk_src);
}

void VulkanCommandList::blit(TextureHandle src, TextureHandle dst, const BlitRegion& srcRegion, const BlitRegion& dstRegion, Filter filter)
{
	AKA_ASSERT(m_recording, "Trying to record something but not recording");
	VulkanTexture* vk_src = device->getVk<VulkanTexture>(src);
	VulkanTexture* vk_dst = device->getVk<VulkanTexture>(dst);

	vk_dst->blitFrom(vk_command, vk_src, srcRegion, dstRegion, filter);
}

void VulkanCommandList::beginMarker(const char* name, const float* color)
{
	VkDebugUtilsLabelEXT label{};
	label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	memcpy(label.color, color, sizeof(float) * 4);
	label.pLabelName = name;

	vkCmdBeginDebugUtilsLabelEXT(vk_command, &label);
}

void VulkanCommandList::endMarker()
{
	vkCmdEndDebugUtilsLabelEXT(vk_command);
}

VkCommandBuffer VulkanCommandList::createSingleTime(const char* debugName, VkDevice device, VkCommandPool commandPool)
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

	VkDebugUtilsLabelEXT label{};
	label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	label.color[0] = 0.f;
	label.color[0] = 0.f;
	label.color[0] = 0.f;
	label.color[0] = 1.f;
	label.pLabelName = debugName;
	vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &label);

	return commandBuffer;
}

void VulkanCommandList::endSingleTime(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue graphicsQueue)
{
	vkCmdEndDebugUtilsLabelEXT(commandBuffer);
	VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VK_CHECK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK_RESULT(vkQueueWaitIdle(graphicsQueue));

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

VkQueue VulkanGraphicDevice::getVkQueue(QueueType type)
{
	AKA_ASSERT(EnumToIndex(type) < EnumCount<QueueType>(), "Invalid queue");
	return m_context.queues[EnumToIndex(type)].queue;
}

VkQueue VulkanGraphicDevice::getVkPresentQueue()
{
	return m_context.presentQueue.queue;
}

uint32_t VulkanGraphicDevice::getVkQueueIndex(QueueType type)
{
	AKA_ASSERT(EnumToIndex(type) < EnumCount<QueueType>(), "Invalid queue");
	return m_context.queues[EnumToIndex(type)].familyIndex;
}

uint32_t VulkanGraphicDevice::getVkPresentQueueIndex()
{
	return m_context.presentQueue.familyIndex;
}

};
};

#endif