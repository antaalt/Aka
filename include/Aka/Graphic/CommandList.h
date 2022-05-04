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

enum class SubmitFlag
{
	None = 0,
	Primary = (1 << 0), // is it a primary command buffer ?
	// TODO instead, pass a main command list in frame
	// TODO BETTER create a useless & complex dependency system that will take me 3 years of my life
};

struct AKA_NO_VTABLE CommandList
{
	virtual ~CommandList() {}

	virtual void begin() = 0;
	virtual void end() = 0;

	virtual void reset() = 0;

	virtual void beginRenderPass(const Framebuffer* framebuffer, const ClearState& clear) = 0;
	virtual void endRenderPass() = 0;

	virtual void bindPipeline(const Pipeline* handle) = 0;
	virtual void bindDescriptorSet(uint32_t index, DescriptorSetHandle material) = 0;
	virtual void bindDescriptorSets(DescriptorSetHandle* material, uint32_t count) = 0;

	virtual void bindVertexBuffer(const Buffer* const* handle, uint32_t binding, uint32_t bindingCount, const uint32_t* offsets) = 0;
	virtual void bindIndexBuffer(const Buffer* buffer, IndexFormat format, uint32_t offset) = 0;

	virtual void clear(ClearMask mask, const float* color, float depth, uint32_t stencil) = 0;

	virtual void draw(uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceCount = 1) = 0;
	virtual void drawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceCount = 1) = 0;
	virtual void dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ) = 0;

	virtual void copy(const Texture* src, const Texture* dst) = 0;

	virtual void blit(const Texture* src, const Texture* dst, BlitRegion srcRegion, BlitRegion dstRegion, Filter filter) = 0;
};

};
};