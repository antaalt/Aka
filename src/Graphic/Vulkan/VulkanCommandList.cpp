#include "VulkanCommandList.h"

#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "VulkanGraphicDevice.h"

#if defined(AKA_USE_VULKAN)

namespace aka {
namespace gfx {

CommandEncoder* VulkanGraphicDevice::acquireCommandEncoder(QueueType queue)
{
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.commandPool = m_context.commandPool[EnumToIndex(queue)];
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer cmd = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_context.device, &allocateInfo, &cmd));

	return m_commandEncoderPool.acquire(this, cmd, queue, true);
}
void VulkanGraphicDevice::execute(const char* _name, std::function<void(CommandList&)> callback, QueueType queue, bool async)
{
	CommandEncoder* cmd = acquireCommandEncoder(queue);
	cmd->record([c = move(callback), _name](CommandList& cmd) {
		ScopedCmdMarker marker(cmd, _name, 1.f, 1.f, 1.f, 1.f);
		c(cmd);
	});
	if (async)
	{
		// Delay command destruction
		AKA_ASSERT(m_copyFenceCounter != InvalidFenceValue, "CopyFenceCounter reached max.");
		submit(cmd, m_copyFenceHandle, m_copyFenceCounter, m_copyFenceCounter + 1);
		++m_copyFenceCounter;
		m_commandEncoderToRelease.append(dynamic_cast<VulkanCommandEncoder*>(cmd));
	}
	else
	{
		// Block & wait for execution to complete
		submit(cmd);
		wait(queue);
		release(cmd);
	}
}

void VulkanGraphicDevice::executeVk(const char* _name, std::function<void(VulkanCommandList&)> callback, QueueType queue, bool async)
{
	execute(_name, [vk_callback = move(callback)](CommandList& command) {
		// We cant use reinterpret_cast because of virtual & multiple inheritance.
		VulkanCommandList& vk_command = dynamic_cast<VulkanCommandList&>(command);
		vk_callback(vk_command);
	}, queue, async);
}

void VulkanGraphicDevice::release(CommandEncoder* cmd)
{
	VulkanCommandEncoder* vk_cmd = reinterpret_cast<VulkanCommandEncoder*>(cmd);
	VkCommandBuffer vk_command = vk_cmd->getVkCommandBuffer();
	vkFreeCommandBuffers(m_context.device, getVkCommandPool(vk_cmd->getQueueType()), 1, &vk_command);

	m_commandEncoderPool.release(vk_cmd);
}

void VulkanGraphicDevice::submit(CommandEncoder* command, FenceHandle handle, FenceValue waitValue, FenceValue signalValue)
{
	VulkanCommandEncoder* vk_command = reinterpret_cast<VulkanCommandEncoder*>(command);
	VulkanFence* vk_fence = getVk<VulkanFence>(handle);

	VkQueue vk_queue = getVkQueue(vk_command->getQueueType());
	VkCommandBuffer vk_cmd= vk_command->getVkCommandBuffer();

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

	VkTimelineSemaphoreSubmitInfo timelineInfo;
	timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	timelineInfo.pNext = NULL;
	timelineInfo.waitSemaphoreValueCount = (waitValue != InvalidFenceValue) ? 1 : 0;
	timelineInfo.pWaitSemaphoreValues = (waitValue != InvalidFenceValue) ? &waitValue : nullptr;
	timelineInfo.signalSemaphoreValueCount = (signalValue != InvalidFenceValue) ? 1 : 0;
	timelineInfo.pSignalSemaphoreValues = (signalValue != InvalidFenceValue) ? &signalValue : nullptr;

	uint32_t waitSemaphoreCount = 0;
	uint32_t signalSemaphoreCount = 0;
	VkSemaphore waitSemaphore[1] = { VK_NULL_HANDLE };
	VkSemaphore signalSemaphore[1] = { VK_NULL_HANDLE };
	if (handle != FenceHandle::null)
	{
		waitSemaphoreCount = waitValue != InvalidFenceValue ? 1 : 0;
		signalSemaphoreCount = signalValue != InvalidFenceValue ? 1 : 0;
		waitSemaphore[0] = vk_fence->vk_sempahore;
		signalSemaphore[0] = vk_fence->vk_sempahore; // TODO handle invalid values...
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = &timelineInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vk_cmd;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = signalSemaphoreCount;
	submitInfo.pSignalSemaphores = signalSemaphore;
	submitInfo.waitSemaphoreCount = waitSemaphoreCount;
	submitInfo.pWaitSemaphores = waitSemaphore;

	static const char* s_commandName[EnumCount<QueueType>()] = {
		"Graphic queue",
		"Compute queue",
		"Copy queue",
	};
	color4f markerColor(0.8f, 0.8f, 0.8f, 1.f);
	beginMarker(vk_command->getQueueType(), s_commandName[EnumToIndex(vk_command->getQueueType())], markerColor.data);
	VK_CHECK_RESULT(vkQueueSubmit(vk_queue, 1, &submitInfo, VK_NULL_HANDLE));
	endMarker(vk_command->getQueueType());
}

void VulkanGraphicDevice::wait(QueueType queue)
{
	VK_CHECK_RESULT(vkQueueWaitIdle(getVkQueue(queue)));
}

FrameIndex VulkanGraphicDevice::getFrameIndex(SwapchainHandle handle, FrameHandle frame)
{
	VulkanSwapchain* vk_swapchain = getVk<VulkanSwapchain>(handle);
	return vk_swapchain->getVkFrameIndex(frame);
}

CommandList* VulkanGraphicDevice::getCopyCommandList(SwapchainHandle handle, FrameHandle frame)
{
	VulkanFrame* vk_frame = getVk<VulkanFrame>(frame);
	return &vk_frame->mainCommandEncoders[EnumToIndex(QueueType::Copy)]->getCommandList();
}

CommandList* VulkanGraphicDevice::getGraphicCommandList(SwapchainHandle handle, FrameHandle frame)
{
	VulkanFrame* vk_frame = getVk<VulkanFrame>(frame);
	return &vk_frame->mainCommandEncoders[EnumToIndex(QueueType::Graphic)]->getCommandList();
}

CommandList* VulkanGraphicDevice::getComputeCommandList(SwapchainHandle handle, FrameHandle frame)
{
	VulkanFrame* vk_frame = getVk<VulkanFrame>(frame);
	return &vk_frame->mainCommandEncoders[EnumToIndex(QueueType::Compute)]->getCommandList();
}

VulkanCommandList::VulkanCommandList(VulkanGraphicDevice* device, QueueType queueType, VkCommandBuffer cmd) :
	VulkanGenericCommandList(device, queueType, cmd)
{
}

VulkanCommandEncoder::VulkanCommandEncoder(VulkanGraphicDevice* device, VkCommandBuffer command, QueueType queue, bool oneTimeSubmit) :
	m_device(device),
	m_commandList(device, queue, command),
	m_oneTimeSubmit(oneTimeSubmit),
	m_queue(queue)
{
}
void VulkanCommandEncoder::record(std::function<void(CommandList&)> callback)
{
	CommandList* cmd = begin();
	callback(*cmd);
	end(cmd);
}

CommandList* VulkanCommandEncoder::begin()
{
	VK_CHECK_RESULT(vkResetCommandBuffer(m_commandList.getVkCommandBuffer(), 0)); //VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = m_oneTimeSubmit ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VK_CHECK_RESULT(vkBeginCommandBuffer(m_commandList.getVkCommandBuffer(), &beginInfo));

	return &m_commandList;
}

void VulkanCommandEncoder::end(CommandList* cmd)
{
	// TODO: can be false with reinterpret_cast ?
	AKA_ASSERT(cmd == &m_commandList, "Passing invalid command to end.");

	VK_CHECK_RESULT(vkEndCommandBuffer(m_commandList.getVkCommandBuffer()));
}

void VulkanCommandEncoder::reset()
{
	vkResetCommandBuffer(m_commandList.getVkCommandBuffer(), VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void VulkanCommandList::executeRenderPass(RenderPassHandle renderPass, FramebufferHandle framebuffer, const ClearState& clear, std::function<void(RenderPassCommandList&)> callback)
{
	VulkanFramebuffer* vk_framebuffer = getDevice()->getVk<VulkanFramebuffer>(framebuffer);
	VulkanRenderPass* vk_renderPass = getDevice()->getVk<VulkanRenderPass>(renderPass);

	Vector<VkClearValue> clearValues(vk_framebuffer->count);
	for (uint32_t i = 0; i < vk_framebuffer->count; i++)
	{
		memcpy(clearValues[i].color.float32, clear.colors[i], sizeof(float) * 4);
	}
	if (vk_framebuffer->hasDepthStencil())
	{
		VkClearValue depthClear{};
		depthClear.depthStencil = VkClearDepthStencilValue{ clear.depth, clear.stencil };
		clearValues.append(depthClear);
	}

	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = vk_renderPass->vk_renderpass;
	beginInfo.framebuffer = vk_framebuffer->vk_framebuffer;
	beginInfo.renderArea.offset = VkOffset2D{ 0, 0 };
	beginInfo.renderArea.extent = VkExtent2D{ vk_framebuffer->width, vk_framebuffer->height };
	beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	beginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(getVkCommandBuffer(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VulkanRenderPassCommandList passList(*this, vk_renderPass, vk_framebuffer);
	callback(passList);

	vkCmdEndRenderPass(getVkCommandBuffer());
}

void VulkanCommandList::executeComputePass(std::function<void(ComputePassCommandList&)> callback)
{
	VulkanComputePassCommandList passList(*this);
	callback(passList);
}

void VulkanRenderPassCommandList::bindPipeline(GraphicPipelineHandle pipeline)
{
	m_boundPipeline = getDevice()->getVk<VulkanGraphicPipeline>(pipeline);
	VkCommandBuffer vk_command = getVkCommandBuffer();
	VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	vkCmdBindPipeline(vk_command, bindPoint, m_boundPipeline->vk_pipeline);
}

void VulkanComputePassCommandList::bindPipeline(ComputePipelineHandle pipeline)
{
	m_boundPipeline = getDevice()->getVk<VulkanComputePipeline>(pipeline);
	VkCommandBuffer vk_command = getVkCommandBuffer();
	VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
	vkCmdBindPipeline(vk_command, bindPoint, m_boundPipeline->vk_pipeline);
}

void VulkanComputePassCommandList::bindDescriptorSet(uint32_t index, DescriptorSetHandle set)
{
	AKA_ASSERT(m_boundPipeline != nullptr, "No pipeline bound");

	VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
	const DescriptorSet* descriptorSet = getDevice()->get(set);
	if (descriptorSet->bindings.count > 0)
	{
		VkPipelineLayout vk_pipelineLayout = m_boundPipeline->vk_pipelineLayout;
		const VulkanDescriptorSet* vk_descriptor = reinterpret_cast<const VulkanDescriptorSet*>(descriptorSet);
		vkCmdBindDescriptorSets(getVkCommandBuffer(), bindPoint, vk_pipelineLayout, index, 1, &vk_descriptor->vk_descriptorSet, 0, nullptr);
	}
}
void VulkanRenderPassCommandList::bindDescriptorSets(const DescriptorSetHandle* sets, uint32_t count)
{
	if (count < 1)
		return;
	AKA_ASSERT(m_boundPipeline != nullptr, "No pipeline bound");

	VkPipelineBindPoint vk_bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	VkDescriptorSet vk_sets[ShaderMaxSetCount]{};
	VkPipelineLayout vk_pipelineLayout = m_boundPipeline->vk_pipelineLayout;
	for (uint32_t i = 0; i < count; i++)
	{
		const DescriptorSet* descriptorSet = getDevice()->get(sets[i]);
		if (descriptorSet->bindings.count > 0)
		{
			const VulkanDescriptorSet* vk_set = reinterpret_cast<const VulkanDescriptorSet*>(descriptorSet);
			vk_sets[i] = vk_set->vk_descriptorSet;
		}
		else
		{
			vk_sets[i] = VK_NULL_HANDLE;
		}
	}
	vkCmdBindDescriptorSets(getVkCommandBuffer(), vk_bindPoint, vk_pipelineLayout, 0, count, vk_sets, 0, nullptr);
}

VulkanComputePassCommandList::VulkanComputePassCommandList(VulkanCommandList& cmd) :
	VulkanGenericCommandList(cmd.getDevice(), cmd.getQueueType(), cmd.getVkCommandBuffer())
{
}
VulkanRenderPassCommandList::VulkanRenderPassCommandList(VulkanCommandList& cmd, VulkanRenderPass* renderPass, VulkanFramebuffer* framebuffer) :
	VulkanGenericCommandList(cmd.getDevice(), cmd.getQueueType(), cmd.getVkCommandBuffer()),
	m_renderPass(renderPass),
	m_framebuffer(framebuffer)
{
}
void VulkanRenderPassCommandList::bindDescriptorSet(uint32_t index, DescriptorSetHandle set)
{
	AKA_ASSERT(m_boundPipeline != nullptr, "No pipeline bound");

	VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	const DescriptorSet* descriptorSet = getDevice()->get(set);
	if (descriptorSet->bindings.count > 0)
	{
		VkPipelineLayout vk_pipelineLayout = m_boundPipeline->vk_pipelineLayout;
		const VulkanDescriptorSet* vk_descriptor = reinterpret_cast<const VulkanDescriptorSet*>(descriptorSet);
		vkCmdBindDescriptorSets(getVkCommandBuffer(), bindPoint, vk_pipelineLayout, index, 1, &vk_descriptor->vk_descriptorSet, 0, nullptr);
	}
}
void VulkanComputePassCommandList::bindDescriptorSets(const DescriptorSetHandle* sets, uint32_t count)
{
	if (count < 1)
		return;
	AKA_ASSERT(m_boundPipeline != nullptr, "No pipeline bound");

	VkPipelineBindPoint vk_bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
	VkDescriptorSet vk_sets[ShaderMaxSetCount]{};
	VkPipelineLayout vk_pipelineLayout = m_boundPipeline->vk_pipelineLayout;
	for (uint32_t i = 0; i < count; i++)
	{
		const DescriptorSet* descriptorSet = getDevice()->get(sets[i]);
		if (descriptorSet->bindings.count > 0)
		{
			const VulkanDescriptorSet* vk_set = reinterpret_cast<const VulkanDescriptorSet*>(descriptorSet);
			vk_sets[i] = vk_set->vk_descriptorSet;
		}
		else
		{
			vk_sets[i] = VK_NULL_HANDLE;
		}
	}
	vkCmdBindDescriptorSets(getVkCommandBuffer(), vk_bindPoint, vk_pipelineLayout, 0, count, vk_sets, 0, nullptr);
}
void VulkanGenericCommandList::transition(TextureHandle texture, ResourceAccessType src, ResourceAccessType dst)
{
	VulkanTexture* vk_texture = getDevice()->getVk<VulkanTexture>(texture);
	VulkanTexture::transitionImageLayout(
		getVkCommandBuffer(),
		getQueueType(),
		vk_texture->vk_image,
		src,
		dst, 
		vk_texture->format
	);
}
void VulkanGenericCommandList::transition(BufferHandle buffer, ResourceAccessType src, ResourceAccessType dst)
{
	VulkanBuffer* vk_buffer = getDevice()->getVk<VulkanBuffer>(buffer);
	VulkanBuffer::transitionBuffer(
		getVkCommandBuffer(),
		getQueueType(),
		vk_buffer->vk_buffer,
		vk_buffer->size,
		0,
		src,
		dst
	);
}
void VulkanRenderPassCommandList::bindVertexBuffer(uint32_t binding, const BufferHandle buffers, uint32_t offset)
{
	bindVertexBuffers(binding, 1, &buffers, &offset);
}
void VulkanRenderPassCommandList::bindVertexBuffers(uint32_t binding, uint32_t bindingCount, const BufferHandle* buffers, const uint32_t* offsets)
{
	VkBuffer vk_buffers[VertexMaxAttributeCount]{};
	VkDeviceSize vk_offsets[VertexMaxAttributeCount]{};
	for (size_t i = 0; i < bindingCount; i++)
	{
		VulkanBuffer* vk_buffer = getDevice()->getVk<VulkanBuffer>(buffers[i]);
		vk_buffers[i] = vk_buffer->vk_buffer;
		vk_offsets[i] = offsets ? offsets[i] : 0;
	}
	vkCmdBindVertexBuffers(getVkCommandBuffer(), binding, bindingCount, vk_buffers, vk_offsets);
}
void VulkanRenderPassCommandList::bindIndexBuffer(BufferHandle buffer, IndexFormat format, uint32_t offset)
{
	VulkanBuffer* vk_buffer = getDevice()->getVk<VulkanBuffer>(buffer);
	VkBuffer buffers = vk_buffer->vk_buffer;
	VkIndexType indexType = VulkanContext::tovk(format);
	vkCmdBindIndexBuffer(getVkCommandBuffer(), buffers, offset, indexType);
}
void VulkanRenderPassCommandList::clearAll(ClearMask mask, const float* color, float depth, uint32_t stencil)
{
	Vector<VkClearAttachment> attachments;
	if (!isNull(mask & ClearMask::Color))
	{
		for (uint32_t i = 0; i < m_framebuffer->count; i++)
		{
			const Texture* tex = getDevice()->get(m_framebuffer->colors[i].texture);
			AKA_ASSERT(Texture::isColor(tex->format), "Texture is not color.");
			VkClearAttachment att{};
			att.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			att.clearValue.color = VkClearColorValue{ color[0], color[1], color[2], color[3] };
			att.colorAttachment = i;
			attachments.append(att);
		}
	}
	if (m_framebuffer->depth.texture != gfx::TextureHandle::null)
	{
		const Texture* tex = getDevice()->get(m_framebuffer->depth.texture);
		VkImageAspectFlags aspect = 0;
		if (Texture::hasDepth(tex->format))
			aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
		if (Texture::hasStencil(tex->format))
			aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
		AKA_ASSERT(aspect != 0, "Invalid depth image.");
		VkClearAttachment att{};
		att.aspectMask = aspect;
		att.clearValue.depthStencil = VkClearDepthStencilValue{ depth, stencil };
		att.colorAttachment = m_framebuffer->count; // depth is last
		attachments.append(att);
	}

	VkClearRect clearRect{};
	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;
	clearRect.rect.extent = VkExtent2D{ m_framebuffer->width, m_framebuffer->height };
	clearRect.rect.offset = VkOffset2D{ 0, 0 };
	vkCmdClearAttachments(getVkCommandBuffer(), (uint32_t)attachments.size(), attachments.data(), 1, &clearRect);
}

void VulkanRenderPassCommandList::clearColor(uint32_t slot, const float* color)
{
	AKA_ASSERT(slot < m_framebuffer->count, "Slot out of bounds.");

	const Texture* tex = getDevice()->get(m_framebuffer->colors[slot].texture);
	AKA_ASSERT(Texture::isColor(tex->format), "Texture is not color.");
	VkClearAttachment att{};
	att.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	att.clearValue.color = VkClearColorValue{ color[0], color[1], color[2], color[3] };
	att.colorAttachment = slot;

	VkClearRect clearRect{};
	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;
	clearRect.rect.extent = VkExtent2D{ m_framebuffer->width, m_framebuffer->height };
	clearRect.rect.offset = VkOffset2D{ 0, 0 };
	vkCmdClearAttachments(getVkCommandBuffer(), 1, &att, 1, &clearRect);
}
void VulkanRenderPassCommandList::clearDepthStencil(ClearMask mask, float depth, uint32_t stencil)
{
	AKA_ASSERT(this->m_framebuffer->depth.texture != gfx::TextureHandle::null, "No depth texture in framebuffer.");

	const Texture* tex = getDevice()->get(m_framebuffer->depth.texture);
	VkImageAspectFlags aspect = 0;
	if (Texture::hasDepth(tex->format))
		aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
	if (Texture::hasStencil(tex->format))
		aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
	AKA_ASSERT(aspect != 0, "Invalid depth image.");
	VkClearAttachment att{};
	att.aspectMask = aspect;
	att.clearValue.depthStencil = VkClearDepthStencilValue{ depth, stencil };
	att.colorAttachment = this->m_framebuffer->count; // depth is last

	VkClearRect clearRect{};
	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;
	clearRect.rect.extent = VkExtent2D{ m_framebuffer->width, m_framebuffer->height };
	clearRect.rect.offset = VkOffset2D{ 0, 0 };
	vkCmdClearAttachments(getVkCommandBuffer(), 1, &att, 1, &clearRect);
}

void VulkanGenericCommandList::clearColor(TextureHandle handle, const float* color)
{
	VulkanTexture* texture = getDevice()->getVk<VulkanTexture>(handle);

	VkClearColorValue values{};
	memcpy(values.float32, color, sizeof(float) * 4);
	VkImageSubresourceRange range{};
	range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.layerCount = 1;
	range.levelCount = 1;
	vkCmdClearColorImage(getVkCommandBuffer(), texture->vk_image, VulkanContext::tovk(ResourceAccessType::Resource, texture->format), &values, 1, &range);
}

void VulkanGenericCommandList::clearDepthStencil(TextureHandle handle, float depth, uint32_t stencil)
{
	VulkanTexture* texture = getDevice()->getVk<VulkanTexture>(handle);

	VkClearDepthStencilValue values{};
	values.depth = depth;
	values.stencil = stencil;
	VkImageSubresourceRange range{};
	range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.layerCount = 1;
	range.levelCount = 1;
	vkCmdClearDepthStencilImage(getVkCommandBuffer(), texture->vk_image, VulkanContext::tovk(ResourceAccessType::Resource, texture->format), &values, 1, &range);
}

void VulkanRenderPassCommandList::push(uint32_t offset, uint32_t range, const void* data, ShaderMask mask)
{
	AKA_ASSERT(m_boundPipeline != nullptr, "No pipeline bound");
	VkPipelineLayout layout = m_boundPipeline->vk_pipelineLayout;
	vkCmdPushConstants(getVkCommandBuffer(), layout, VulkanContext::tovk(mask), offset, range, data);
}

void VulkanComputePassCommandList::push(uint32_t offset, uint32_t range, const void* data, ShaderMask mask)
{
	AKA_ASSERT(m_boundPipeline != nullptr, "No pipeline bound");
	VkPipelineLayout layout = m_boundPipeline->vk_pipelineLayout;
	vkCmdPushConstants(getVkCommandBuffer(), layout, VulkanContext::tovk(mask), offset, range, data);
}

void VulkanRenderPassCommandList::draw(uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceCount, uint32_t instanceOffset)
{
	AKA_ASSERT(m_boundPipeline != nullptr, "No pipeline bound");
	AKA_ASSERT(getDevice()->getVk<VulkanProgram>(m_boundPipeline->program)->hasShaderStage(ShaderType::Vertex), "Trying draw but vertex shader is missing");
	vkCmdDraw(getVkCommandBuffer(), vertexCount, instanceCount, vertexOffset, instanceOffset);
}
void VulkanRenderPassCommandList::drawIndirect(BufferHandle buffer, uint32_t offset, uint32_t drawCount, uint32_t stride)
{
	AKA_ASSERT(m_boundPipeline != nullptr, "No pipeline bound");
	AKA_ASSERT(getDevice()->getVk<VulkanProgram>(m_boundPipeline->program)->hasShaderStage(ShaderType::Mesh), "Trying draw indexed but vertex shader is missing");
	vkCmdDrawIndirect(getVkCommandBuffer(), getDevice()->getVk<VulkanBuffer>(buffer)->vk_buffer, offset, drawCount, stride);
	//vkCmdDrawIndirectCount(getVkCommandBuffer(), indexCount, instanceCount, indexOffset, vertexOffset, instanceOffset);
}
void VulkanRenderPassCommandList::drawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceCount, uint32_t instanceOffset)
{
	AKA_ASSERT(m_boundPipeline != nullptr, "No pipeline bound");
	AKA_ASSERT(getDevice()->getVk<VulkanProgram>(m_boundPipeline->program)->hasShaderStage(ShaderType::Vertex), "Trying draw indexed but vertex shader is missing");
	vkCmdDrawIndexed(getVkCommandBuffer(), indexCount, instanceCount, indexOffset, vertexOffset, instanceOffset);
}
void VulkanRenderPassCommandList::drawIndexedIndirect(BufferHandle buffer, uint32_t offset, uint32_t drawCount, uint32_t stride)
{
	AKA_ASSERT(m_boundPipeline != nullptr, "No pipeline bound");
	AKA_ASSERT(getDevice()->getVk<VulkanProgram>(m_boundPipeline->program)->hasShaderStage(ShaderType::Vertex), "Trying draw indexed but vertex shader is missing");
	vkCmdDrawIndexedIndirect(getVkCommandBuffer(), getDevice()->getVk<VulkanBuffer>(buffer)->vk_buffer, offset, drawCount, stride);
	//vkCmdDrawIndexedIndirectCount(getVkCommandBuffer(), indexCount, instanceCount, indexOffset, vertexOffset, instanceOffset);
}
void VulkanRenderPassCommandList::drawMeshTasks(uint32_t groupX, uint32_t groupY, uint32_t groupZ)
{
	AKA_ASSERT(m_boundPipeline != nullptr, "No pipeline bound");
	AKA_ASSERT(getDevice()->getVk<VulkanProgram>(m_boundPipeline->program)->hasShaderStage(ShaderType::Mesh), "Trying draw mesh task but vertex shader is missing");
	vkCmdDrawMeshTasksEXT(getVkCommandBuffer(), groupX, groupY, groupZ);
}
void VulkanRenderPassCommandList::drawMeshTasksIndirect(BufferHandle buffer, uint32_t offset, uint32_t drawCount, uint32_t stride)
{
	AKA_ASSERT(m_boundPipeline != nullptr, "No pipeline bound");
	AKA_ASSERT(getDevice()->getVk<VulkanProgram>(m_boundPipeline->program)->hasShaderStage(ShaderType::Mesh), "Trying draw mesh task but vertex shader is missing");
	vkCmdDrawMeshTasksIndirectEXT(getVkCommandBuffer(), getDevice()->getVk<VulkanBuffer>(buffer)->vk_buffer, offset, drawCount, stride);
	//vkCmdDrawMeshTasksIndirectCountEXT(getVkCommandBuffer(), groupX, groupY, groupZ);
}
void VulkanComputePassCommandList::dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ) 
{
	AKA_ASSERT(m_boundPipeline != nullptr, "No pipeline bound");
	AKA_ASSERT(getDevice()->getVk<VulkanProgram>(m_boundPipeline->program)->hasShaderStage(ShaderType::Compute), "Trying dispatch but compute shader is missing");
	vkCmdDispatch(getVkCommandBuffer(), groupX, groupY, groupZ);
}
void VulkanComputePassCommandList::dispatchIndirect(BufferHandle buffer, uint32_t offset)
{
	AKA_ASSERT(m_boundPipeline != nullptr, "No pipeline bound");
	AKA_ASSERT(getDevice()->getVk<VulkanProgram>(m_boundPipeline->program)->hasShaderStage(ShaderType::Compute), "Trying dispatch but compute shader is missing");
	vkCmdDispatchIndirect(getVkCommandBuffer(), getDevice()->getVk<VulkanBuffer>(buffer)->vk_buffer, offset);
}

void VulkanGenericCommandList::copy(BufferHandle src, BufferHandle dst, uint32_t srcOffset, uint32_t dstOffset, uint32_t range)
{
	VulkanBuffer* vk_src = getDevice()->getVk<VulkanBuffer>(src);
	VulkanBuffer* vk_dst = getDevice()->getVk<VulkanBuffer>(dst);

	vk_dst->copyFrom(getVkCommandBuffer(), vk_src, srcOffset, dstOffset, range);
}

void VulkanGenericCommandList::copy(TextureHandle src, TextureHandle dst)
{
	VulkanTexture* vk_src = getDevice()->getVk<VulkanTexture>(src);
	VulkanTexture* vk_dst = getDevice()->getVk<VulkanTexture>(dst);

	vk_dst->copyFrom(getVkCommandBuffer(), vk_src);
}

void VulkanGenericCommandList::blit(TextureHandle src, TextureHandle dst, const BlitRegion& srcRegion, const BlitRegion& dstRegion, Filter filter)
{
	VulkanTexture* vk_src = getDevice()->getVk<VulkanTexture>(src);
	VulkanTexture* vk_dst = getDevice()->getVk<VulkanTexture>(dst);

	vk_dst->blitFrom(getVkCommandBuffer(), vk_src, srcRegion, dstRegion, filter);
}

void VulkanGenericCommandList::beginMarker(const char* name, const float* color)
{
	VkDebugUtilsLabelEXT label{};
	label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	memcpy(label.color, color, sizeof(float) * 4);
	label.pLabelName = name;

	vkCmdBeginDebugUtilsLabelEXT(getVkCommandBuffer(), &label);
}

void VulkanGenericCommandList::endMarker()
{
	vkCmdEndDebugUtilsLabelEXT(getVkCommandBuffer());
}
VkCommandBuffer VulkanGenericCommandList::getVkCommandBuffer()
{
	return m_cmd;
}
VulkanGraphicDevice* VulkanGenericCommandList::getDevice()
{
	return m_device;
}
QueueType VulkanGenericCommandList::getQueueType()
{
	return m_queueType;
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