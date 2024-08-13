#pragma once

#if defined(AKA_USE_VULKAN)

#include <Aka/Graphic/CommandList.h>

#include "VulkanContext.h"

#if defined(AKA_PLATFORM_WINDOWS)
#pragma warning(push)
#pragma warning(disable : 4250) // Inherited via dominance
#endif

namespace aka {
namespace gfx {

class VulkanGraphicDevice;
struct VulkanCommandList;
struct VulkanRenderPass;
struct VulkanFramebuffer;

struct VulkanGenericCommandList : virtual GenericCommandList
{
	VulkanGenericCommandList(VulkanGraphicDevice* device, VkCommandBuffer cmd) : m_device(device), m_cmd(cmd) {}
	void transition(TextureHandle texture, ResourceAccessType src, ResourceAccessType dst) override;
	void transition(BufferHandle buffer, ResourceAccessType src, ResourceAccessType dst) override;

	void copy(BufferHandle src, BufferHandle dst) override;
	void copy(TextureHandle src, TextureHandle dst) override;
	void blit(TextureHandle src, TextureHandle dst, const BlitRegion& srcRegion, const BlitRegion& dstRegion, Filter filter) override;

	void clearColor(TextureHandle handle, const float* color) override;
	void clearDepthStencil(TextureHandle handle, float depth, uint32_t stencil) override;

	void beginMarker(const char* name, const float* color) override;
	void endMarker() override;
public:
	VkCommandBuffer getVkCommandBuffer();
	VulkanGraphicDevice* getDevice();
private:
	VulkanGraphicDevice* m_device;
	VkCommandBuffer m_cmd;
};

struct VulkanComputePassCommandList final : virtual ComputePassCommandList, VulkanGenericCommandList
{
	VulkanComputePassCommandList(VulkanCommandList& cmd);
	void bindDescriptorSet(uint32_t index, DescriptorSetHandle descriptorSet) override;
	void bindDescriptorSets(const DescriptorSetHandle* descriptorSets, uint32_t count) override;

	void push(uint32_t offset, uint32_t range, const void* data, ShaderMask mask) override;

	void bindPipeline(ComputePipelineHandle handle) override;
	void dispatch(uint32_t groupCountX = 1U, uint32_t groupCountY = 1U, uint32_t groupCountZ = 1U)  override;
	void dispatchIndirect(BufferHandle buffer, uint32_t offset = 0U)  override;
private:
	VulkanComputePipeline* m_boundPipeline = nullptr;
};
struct VulkanRenderPassCommandList final : virtual RenderPassCommandList, VulkanGenericCommandList
{
	VulkanRenderPassCommandList(VulkanCommandList& cmd, VulkanRenderPass* renderPass, VulkanFramebuffer* framebuffer);

	void bindDescriptorSet(uint32_t index, DescriptorSetHandle descriptorSet) override;
	void bindDescriptorSets(const DescriptorSetHandle* descriptorSets, uint32_t count) override;

	void push(uint32_t offset, uint32_t range, const void* data, ShaderMask mask) override;

	void bindPipeline(GraphicPipelineHandle handle) override;

	void bindVertexBuffer(uint32_t binding, const BufferHandle handle, uint32_t offsets = 0) override;
	void bindVertexBuffers(uint32_t binding, uint32_t bindingCount, const BufferHandle* handles, const uint32_t* offsets = nullptr) override;
	void bindIndexBuffer(BufferHandle handle, IndexFormat format, uint32_t offset = 0) override;

	void clearAll(ClearMask mask, const float* color, float depth, uint32_t stencil) override;
	void clearColor(uint32_t slot, const float* color) override;
	void clearDepthStencil(ClearMask mask, float depth, uint32_t stencil) override;

	void draw(uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceCount = 1U, uint32_t instanceOffset = 0U) override;
	void drawIndirect(BufferHandle buffer, uint32_t offset = 0U, uint32_t drawCount = 1U, uint32_t stride = sizeof(DrawIndirectCommand)) override;
	void drawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceCount = 1U, uint32_t instanceOffset = 0U) override;
	void drawIndexedIndirect(BufferHandle buffer, uint32_t offset = 0U, uint32_t drawCount = 1U, uint32_t stride = sizeof(DrawIndexedIndirectCommand)) override;
	void drawMeshTasks(uint32_t groupCountX = 1U, uint32_t groupCountY = 1U, uint32_t groupCountZ = 1U) override;
	void drawMeshTasksIndirect(BufferHandle buffer, uint32_t offset = 0U, uint32_t drawCount = 1U, uint32_t stride = sizeof(DrawMeshTasksIndirectCommand)) override;
private:
	VulkanRenderPass* m_renderPass;
	VulkanFramebuffer* m_framebuffer;
	VulkanGraphicPipeline* m_boundPipeline = nullptr;
};
struct VulkanCommandList : virtual CommandList, VulkanGenericCommandList
{
	VulkanCommandList(VulkanGraphicDevice* device, VkCommandBuffer cmd);
	void executeRenderPass(RenderPassHandle renderPass, FramebufferHandle framebuffer, const ClearState& clear, std::function<void(RenderPassCommandList&)> callback) override;
	void executeComputePass(std::function<void(ComputePassCommandList&)> callback) override;
private:
};
struct VulkanCommandEncoder : CommandEncoder
{
	VulkanCommandEncoder(VulkanGraphicDevice* device, VkCommandBuffer command, QueueType queue, bool oneTimeSubmit);

	void record(std::function<void(CommandList&)> callback) override;
	CommandList* begin() override;
	void end(CommandList* cmd) override;

	void reset() override;
public:
	VulkanCommandList& getCommandList() { return m_commandList; }
	QueueType getQueueType() const { return m_queue; }
	VkCommandBuffer getVkCommandBuffer() { return m_commandList.getVkCommandBuffer(); }
	VulkanGraphicDevice* getDevice() { return m_commandList.getDevice(); }
private:
	VulkanGraphicDevice* m_device;
	bool m_oneTimeSubmit;
	QueueType m_queue;
	VulkanCommandList m_commandList;
};

};
};

#if defined(AKA_PLATFORM_WINDOWS)
#pragma warning(pop)
#endif

#endif