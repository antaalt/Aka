#pragma once

#include <stdint.h>

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

struct AKA_NO_VTABLE CommandList
{
	virtual ~CommandList() {}

	virtual void begin() = 0;
	virtual void end() = 0;

	virtual void reset() = 0;

	virtual void beginRenderPass(RenderPassHandle renderPass, FramebufferHandle framebuffer, const ClearState& clear) = 0;
	virtual void endRenderPass() = 0;

	virtual void bindPipeline(GraphicPipelineHandle handle) = 0;
	virtual void bindPipeline(ComputePipelineHandle handle) = 0;
	virtual void bindDescriptorSet(uint32_t index, DescriptorSetHandle material) = 0;
	virtual void bindDescriptorSets(const DescriptorSetHandle* material, uint32_t count) = 0;

	virtual void transition(TextureHandle texture, ResourceAccessType src, ResourceAccessType dst) = 0;
	virtual void transition(BufferHandle buffer, ResourceAccessType src, ResourceAccessType dst) = 0;

	virtual void bindVertexBuffer(uint32_t binding, const BufferHandle handle, uint32_t offsets = 0) = 0;
	virtual void bindVertexBuffers(uint32_t binding, uint32_t bindingCount, const BufferHandle* handles, const uint32_t* offsets = nullptr) = 0;
	virtual void bindIndexBuffer(BufferHandle handle, IndexFormat format, uint32_t offset = 0) = 0;

	virtual void clear(ClearMask mask, const float* color, float depth, uint32_t stencil) = 0;
	virtual void clearColor(TextureHandle handle, const float* color) = 0;
	virtual void clearDepthStencil(TextureHandle handle, float depth, uint32_t stencil) = 0;

	virtual void push(uint32_t offset, uint32_t range, const void* data, ShaderMask mask) = 0;

	virtual void draw(uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceCount = 1U, uint32_t instanceOffset = 0) = 0;
	virtual void drawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceCount = 1U, uint32_t instanceOffset = 0) = 0;
	virtual void dispatch(uint32_t groupCountX = 1U, uint32_t groupCountY = 1U, uint32_t groupCountZ = 1U) = 0;

	virtual void copy(BufferHandle src, BufferHandle dst) = 0;
	virtual void copy(TextureHandle src, TextureHandle dst) = 0;
	virtual void blit(TextureHandle src, TextureHandle dst, const BlitRegion& srcRegion, const BlitRegion& dstRegion, Filter filter) = 0;

	virtual void beginMarker(const char* name, const float* color) = 0;
	virtual void endMarker() = 0;
};

struct ScopedCmdMarker
{
	ScopedCmdMarker(CommandList* cmd, const char* name, float r = 0.8f, float g = 0.8f, float b = 0.8f, float a = 1.f) : m_command(cmd) { color4f c(r, g, b, a); m_command->beginMarker(name, c.data); }
	ScopedCmdMarker(CommandList* cmd, const char* name, const float* colors) : m_command(cmd) { m_command->beginMarker(name, colors); }
	~ScopedCmdMarker() { m_command->endMarker(); }
private:
	CommandList* m_command;
};

};
};