#pragma once

#if defined(AKA_USE_VULKAN)

#include <Aka/Graphic/CommandList.h>

#include "VulkanContext.h"

namespace aka {
namespace gfx {

struct VulkanCommandList : CommandList
{
	VulkanCommandList(VkDevice device, VkCommandBuffer cmd);

	void begin() override;
	void end() override;

	void reset() override;

	void beginRenderPass(FramebufferHandle framebuffer, const ClearState& clear) override;
	void endRenderPass() override;

	void bindPipeline(PipelineHandle pipeline) override;
	void bindDescriptorSet(uint32_t index, DescriptorSetHandle set) override;
	void bindDescriptorSets(DescriptorSetHandle* sets, uint32_t count) override;

	void bindVertexBuffer(BufferHandle* buffers, uint32_t binding, uint32_t bindingCount, const uint32_t* offsets) override;
	void bindIndexBuffer(BufferHandle buffer, IndexFormat format, uint32_t offset) override;

	void clear(ClearMask mask, const float* color, float depth, uint32_t stencil) override;

	void draw(uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceCount = 1) override;
	void drawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceCount = 1) override;
	void dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ) override;

	void copy(TextureHandle src, TextureHandle dst) override;
	void blit(TextureHandle src, TextureHandle dst, BlitRegion srcRegion, BlitRegion dstRegion, Filter filter) override;

	static VkCommandBuffer createSingleTime(VkDevice device, VkCommandPool pool);
	static void endSingleTime(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue graphicQueue);

	const VulkanPipeline* vk_pipeline;
	const DescriptorSet* vk_sets[ShaderBindingState::MaxSetCount];
	const VulkanFramebuffer* vk_framebuffer;
	const VulkanBuffer* vk_indices;
	const VulkanBuffer* vk_vertices;

	VkDevice vk_device;
	VkCommandBuffer vk_command;

	bool m_recording;
};

};
};

#endif