#pragma once

#include <stdint.h>

#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Resource.h>
#include <Aka/Graphic/RenderPass.h>

namespace aka {
namespace gfx {

enum class AttachmentFlag : uint8_t
{
	None					= 0,
	AttachTextureObject		= 1 << 0, // Attach the object instead of the layer
	BackbufferAutoResize	= 1 << 1, // Auto resize attachment following backbuffer size
};
AKA_IMPLEMENT_BITMASK_OPERATOR(AttachmentFlag)

struct Attachment
{
	TextureHandle texture; // Texture used as attachment
	AttachmentFlag flag; // Attachment flag
	uint32_t layer; // Layer of the texture used as attachment (if AttachmentFlag::AttachTextureObject not set)
	uint32_t level; // Level of the mips used as attachment
};

uint32_t getWidth(GraphicDevice* device, const Attachment* colors, uint32_t count, const Attachment* depth);
uint32_t getHeight(GraphicDevice* device, const Attachment* colors, uint32_t count, const Attachment* depth);

struct Framebuffer;
using FramebufferHandle = ResourceHandle<Framebuffer>;

struct Framebuffer : Resource
{
	Framebuffer(const char* name, uint32_t width, uint32_t height, RenderPassHandle handle, const Attachment* colors, uint32_t count, const Attachment* depth);
	virtual ~Framebuffer() {}

	uint32_t width, height; // TODO: Should move this out.

	RenderPassHandle renderPass; // Pass linked to fb

	Attachment colors[FramebufferMaxColorAttachmentCount];
	uint32_t count;
	Attachment depth;

	bool hasDepthStencil() const { return depth.texture != TextureHandle::null; }
};

};
};
