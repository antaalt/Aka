#pragma once

#include <stdint.h>

#include <Aka/Graphic/Texture.h>

namespace aka {

enum class ClearMask : uint8_t
{
	None    = 0,
	Color   = (1 << 0),
	Depth   = (1 << 1),
	Stencil = (1 << 2),
	All = Color | Depth | Stencil
};

bool has(ClearMask flags, ClearMask flag);
ClearMask operator&(ClearMask lhs, ClearMask rhs);
ClearMask operator|(ClearMask lhs, ClearMask rhs);

enum class AttachmentFlag : uint8_t
{
	None = 0,
	AttachTextureObject = (1 << 0), // Attach the object instead of the layer
};

struct Attachment
{
	Texture* texture; // Texture used as attachment
	AttachmentFlag flag; // Attachment flag
	uint32_t layer; // Layer of the texture used as attachment (if AttachmentFlag::AttachTextureObject not set)
	uint32_t level; // Level of the mips used as attachment
};

struct FramebufferState
{
	struct Attachment
	{
		TextureFormat format;
	};
	static constexpr uint32_t MaxColorAttachmentCount = 8;

	uint32_t count; // color attachment count
	Attachment colors[MaxColorAttachmentCount];
	Attachment depth;

	bool hasDepth() const { return depth.format != TextureFormat::Unknown; }
};

struct Framebuffer
{
	uint32_t width, height;
	
	FramebufferState framebuffer;

	Attachment colors[FramebufferState::MaxColorAttachmentCount];
	Attachment depth;

	bool hasDepthStencil() const { return depth.texture != nullptr; }

	static Framebuffer* create(const Attachment* attachments, uint32_t count, const Attachment* depth);
	static void destroy(Framebuffer* framebuffer);
};

};
