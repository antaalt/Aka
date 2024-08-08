#pragma once

#if defined(AKA_USE_VULKAN)

#include <Aka/Graphic/CommandList.h>

#include "VulkanContext.h"

namespace aka {
namespace gfx {

class VulkanGraphicDevice;

struct VulkanCommandList : CommandList
{
	VulkanCommandList();
	VulkanCommandList(VulkanGraphicDevice* device, VkCommandBuffer cmd, QueueType queue, bool oneTimeSubmit);

	void begin() override;
	void end() override;

	void reset() override;

	void beginRenderPass(RenderPassHandle renderPass, FramebufferHandle framebuffer, const ClearState& clear = ClearStateBlack) override;
	void endRenderPass() override;

	void bindPipeline(GraphicPipelineHandle pipeline) override;
	void bindPipeline(ComputePipelineHandle handle) override;
	void bindDescriptorSet(uint32_t index, DescriptorSetHandle set) override;
	void bindDescriptorSets(const DescriptorSetHandle* sets, uint32_t count) override;

	void transition(TextureHandle texture, ResourceAccessType src, ResourceAccessType dst) override;
	void transition(BufferHandle buffer, ResourceAccessType src, ResourceAccessType dst) override;

	void bindVertexBuffer(uint32_t binding, const BufferHandle handle, uint32_t offsets = 0) override;
	void bindVertexBuffers(uint32_t binding, uint32_t bindingCount, const BufferHandle* buffers, const uint32_t* offsets = nullptr) override;
	void bindIndexBuffer(BufferHandle buffer, IndexFormat format, uint32_t offset = 0) override;

	void clearAll(ClearMask mask, const float* color, float depth, uint32_t stencil) override;
	void clearColor(uint32_t slot, const float* color) override;
	void clearDepthStencil(ClearMask mask, float depth, uint32_t stencil) override;

	void clearColor(TextureHandle handle, const float* color) override;
	void clearDepthStencil(TextureHandle handle, float depth, uint32_t stencil) override;

	void push(uint32_t offset, uint32_t range, const void* data, ShaderMask mask) override;

	void draw(uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceCount = 1U, uint32_t instanceOffset = 0U) override;
	void drawIndirect(BufferHandle buffer, uint32_t offset = 0U, uint32_t drawCount = 1U, uint32_t stride = sizeof(DrawIndirectCommand)) override;
	void drawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceCount = 1U, uint32_t instanceOffset = 0U) override;
	void drawIndexedIndirect(BufferHandle buffer, uint32_t offset = 0U, uint32_t drawCount = 1U, uint32_t stride = sizeof(DrawIndexedIndirectCommand)) override;
	void drawMeshTasks(uint32_t groupCountX = 1U, uint32_t groupCountY = 1U, uint32_t groupCountZ = 1U) override;
	void drawMeshTasksIndirect(BufferHandle buffer, uint32_t offset = 0U, uint32_t drawCount = 1U, uint32_t stride = sizeof(DrawMeshTasksIndirectCommand)) override;
	void dispatch(uint32_t groupCountX = 1U, uint32_t groupCountY = 1U, uint32_t groupCountZ = 1U) override;
	void dispatchIndirect(BufferHandle buffer, uint32_t offset = 0U) override;

	void copy(BufferHandle src, BufferHandle dst) override;
	void copy(TextureHandle src, TextureHandle dst) override;
	void blit(TextureHandle src, TextureHandle dst, const BlitRegion& srcRegion, const BlitRegion& dstRegion, Filter filter) override;

	void beginMarker(const char* name, const float* color) override;
	void endMarker() override;

	static VkCommandBuffer createSingleTime(const char* debugname, VkDevice device, VkCommandPool pool);
	static void endSingleTime(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue graphicQueue);

public:
	VkCommandBuffer getVkCommandBuffer() { return vk_command; }
	QueueType getQueueType() { return m_queue; }
	VulkanGraphicDevice* getDevice() { return m_device; }

private:
	const VulkanGraphicPipeline* vk_graphicPipeline;
	const VulkanComputePipeline* vk_computePipeline;
	const DescriptorSet* vk_sets[ShaderMaxSetCount];
	const VulkanFramebuffer* vk_framebuffer;
	const VulkanBuffer* vk_indices;
	const VulkanBuffer* vk_vertices;

private:
	friend class VulkanGraphicDevice;

	VulkanGraphicDevice* m_device;
	VkCommandBuffer vk_command;
	QueueType m_queue;
	bool m_recording;
	bool m_oneTimeSubmit;
};

};
};

#endif