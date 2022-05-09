#pragma once

#include <stdint.h>

#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Resource.h>

namespace aka {
namespace gfx {

struct Framebuffer;
using FramebufferHandle = ResourceHandle<Framebuffer>;

static constexpr uint32_t FramebufferMaxColorAttachmentCount = 8;

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
	AttachTextureObject = (1 << 2), // Attach the object instead of the layer
};

enum class AttachmentOp : uint8_t
{
	Load,
	Clear,
	DontCare,
};
using AttachmentLoadOp = AttachmentOp;
using AttachmentStoreOp = AttachmentOp;

bool has(AttachmentFlag flags, AttachmentFlag flag);
AttachmentFlag operator&(AttachmentFlag lhs, AttachmentFlag rhs);
AttachmentFlag operator|(AttachmentFlag lhs, AttachmentFlag rhs);

struct Attachment
{
	TextureHandle texture; // Texture used as attachment
	AttachmentFlag flag; // Attachment flag
	AttachmentLoadOp loadOp; // LoadOp setting
	uint32_t layer; // Layer of the texture used as attachment (if AttachmentFlag::AttachTextureObject not set)
	uint32_t level; // Level of the mips used as attachment
};

struct FramebufferState
{
	struct Attachment
	{
		TextureFormat format;
		AttachmentLoadOp loadOp;
		// TODO store op
	};

	uint32_t count; // color attachment count
	Attachment colors[FramebufferMaxColorAttachmentCount];
	Attachment depth;

	bool hasDepth() const { return depth.format != TextureFormat::Unknown; }
};

struct Framebuffer : Resource
{
	uint32_t width, height; // TODO this is not really interesting to store this.
	
	FramebufferState framebuffer;

	Attachment colors[FramebufferMaxColorAttachmentCount];
	Attachment depth;

	bool hasDepthStencil() const { return depth.texture.data != nullptr; }

	static FramebufferHandle create(const Attachment* attachments, uint32_t count, const Attachment* depth);
	static void destroy(FramebufferHandle framebuffer);
};

bool operator<(const FramebufferState& lhs, const FramebufferState& rhs);
bool operator>(const FramebufferState& lhs, const FramebufferState& rhs);
bool operator==(const FramebufferState& lhs, const FramebufferState& rhs);
bool operator!=(const FramebufferState& lhs, const FramebufferState& rhs);

};
};

template <>
struct std::hash<aka::gfx::FramebufferState>
{
	size_t operator()(const aka::gfx::FramebufferState& data) const
	{
		size_t hash = 0;
		aka::hashCombine(hash, data.count);
		for (size_t i = 0; i < data.count; i++)
		{
			aka::hashCombine(hash, aka::EnumToIntegral(data.colors[i].format));
			aka::hashCombine(hash, aka::EnumToIntegral(data.colors[i].loadOp));
		}
		aka::hashCombine(hash, aka::EnumToIntegral(data.depth.format));
		aka::hashCombine(hash, aka::EnumToIntegral(data.depth.loadOp));
		return hash;
	}
};
