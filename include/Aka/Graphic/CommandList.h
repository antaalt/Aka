#pragma once

#include <stdint.h>
#include <functional>

#include <Aka/Core/Config.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Sampler.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/Pipeline.h>
#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Graphic/DescriptorSet.h>

namespace aka {
namespace gfx {

enum class QueueType : uint8_t
{
	Unknown,

	Graphic,
	Compute,
	Copy,

	First = Graphic,
	Last = Copy,

	Default = Graphic
};

struct BlitRegion
{
	int32_t x, y, z;
	uint32_t w, h, d;
	uint32_t layer;
	uint32_t layerCount;
	uint32_t mipLevel;
};

struct DispatchIndirectCommand
{
	uint32_t x;
	uint32_t y;
	uint32_t z;
};

struct DrawIndexedIndirectCommand
{
	uint32_t indexCount;
	uint32_t instanceCount;
	uint32_t firstIndex;
	int32_t  vertexOffset;
	uint32_t firstInstance;
};

struct DrawIndirectCommand
{
	uint32_t vertexCount;
	uint32_t instanceCount;
	uint32_t firstVertex;
	uint32_t firstInstance;
};

struct DrawMeshTasksIndirectCommand
{
	uint32_t x;
	uint32_t y;
	uint32_t z;
};

struct GenericCommandList
{
	virtual ~GenericCommandList() {}

	virtual void transition(TextureHandle texture, ResourceAccessType src, ResourceAccessType dst) = 0;
	virtual void transition(BufferHandle buffer, ResourceAccessType src, ResourceAccessType dst) = 0;

	virtual void copy(BufferHandle src, BufferHandle dst, uint32_t srcOffset = 0U, uint32_t dstOffset = 0U, uint32_t range = ~0U) = 0;
	virtual void copy(TextureHandle src, TextureHandle dst) = 0;
	virtual void blit(TextureHandle src, TextureHandle dst, const BlitRegion& srcRegion, const BlitRegion& dstRegion, Filter filter) = 0;

	virtual void clearColor(TextureHandle handle, const float* color) = 0;
	virtual void clearDepthStencil(TextureHandle handle, float depth, uint32_t stencil) = 0;

	virtual void beginMarker(const char* name, const float* color) = 0;
	virtual void endMarker() = 0;
};


struct ComputePassCommandList : virtual GenericCommandList
{
	virtual ~ComputePassCommandList() {}

	virtual void bindDescriptorSet(uint32_t index, DescriptorSetHandle descriptorSet) = 0;
	virtual void bindDescriptorSets(const DescriptorSetHandle* descriptorSets, uint32_t count) = 0;

	virtual void push(uint32_t offset, uint32_t range, const void* data, ShaderMask mask) = 0;

	virtual void bindPipeline(ComputePipelineHandle handle) = 0;

	virtual void dispatch(uint32_t groupCountX = 1U, uint32_t groupCountY = 1U, uint32_t groupCountZ = 1U) = 0;
	virtual void dispatchIndirect(BufferHandle buffer, uint32_t offset = 0U) = 0;
};

struct RenderPassCommandList : virtual GenericCommandList
{
	virtual ~RenderPassCommandList() {}

	virtual void bindDescriptorSet(uint32_t index, DescriptorSetHandle descriptorSet) = 0;
	virtual void bindDescriptorSets(const DescriptorSetHandle* descriptorSets, uint32_t count) = 0;

	virtual void push(uint32_t offset, uint32_t range, const void* data, ShaderMask mask) = 0;

	virtual void bindPipeline(GraphicPipelineHandle handle) = 0;

	virtual void bindVertexBuffer(uint32_t binding, const BufferHandle handle, uint32_t offsets = 0) = 0;
	virtual void bindVertexBuffers(uint32_t binding, uint32_t bindingCount, const BufferHandle* handles, const uint32_t* offsets = nullptr) = 0;
	virtual void bindIndexBuffer(BufferHandle handle, IndexFormat format, uint32_t offset = 0) = 0;
 
	virtual void clearAll(ClearMask mask, const float* color, float depth, uint32_t stencil) = 0;
	virtual void clearColor(uint32_t slot, const float* color) = 0;
	virtual void clearDepthStencil(ClearMask mask, float depth, uint32_t stencil) = 0;
	  
	virtual void draw(uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceCount = 1U, uint32_t instanceOffset = 0U) = 0;
	virtual void drawIndirect(BufferHandle buffer, uint32_t offset = 0U, uint32_t drawCount = 1U, uint32_t stride = sizeof(DrawIndirectCommand)) = 0;
	virtual void drawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceCount = 1U, uint32_t instanceOffset = 0U) = 0;
	virtual void drawIndexedIndirect(BufferHandle buffer, uint32_t offset = 0U, uint32_t drawCount = 1U, uint32_t stride = sizeof(DrawIndexedIndirectCommand)) = 0;
	virtual void drawMeshTasks(uint32_t groupCountX = 1U, uint32_t groupCountY = 1U, uint32_t groupCountZ = 1U) = 0;
	virtual void drawMeshTasksIndirect(BufferHandle buffer, uint32_t offset = 0U, uint32_t drawCount = 1U, uint32_t stride = sizeof(DrawMeshTasksIndirectCommand)) = 0;
};

struct CommandList : virtual GenericCommandList
{
	virtual ~CommandList() {}

	virtual void executeRenderPass(RenderPassHandle renderPass, FramebufferHandle framebuffer, const ClearState& clear, std::function<void(RenderPassCommandList&)> callback) = 0;
	virtual void executeComputePass(std::function<void(ComputePassCommandList&)> callback) = 0;
};

struct CommandEncoder
{
	virtual ~CommandEncoder() {}

	virtual void record(std::function<void(CommandList&)> callback) = 0;

	virtual CommandList* begin() = 0;
	virtual void end(CommandList*) = 0;

	virtual void reset() = 0;
};

struct ScopedCmdMarker
{
	ScopedCmdMarker(CommandList& cmd, const char* name, float r = 0.8f, float g = 0.8f, float b = 0.8f, float a = 1.f) : m_command(cmd) { color4f c(r, g, b, a); m_command.beginMarker(name, c.data); }
	ScopedCmdMarker(CommandList& cmd, const char* name, const float* colors) : m_command(cmd) { m_command.beginMarker(name, colors); }
	~ScopedCmdMarker() { m_command.endMarker(); }
private:
	CommandList& m_command;
};

};
};