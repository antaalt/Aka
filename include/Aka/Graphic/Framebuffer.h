#pragma once

#include <stdint.h>

#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Resource.h>

namespace aka {
namespace gfx {

static constexpr uint32_t FramebufferMaxColorAttachmentCount = 8;

enum class ClearMask : uint8_t
{
	None    = 0,
	Color   = (1 << 0),
	Depth   = (1 << 1),
	Stencil = (1 << 2),
	All = Color | Depth | Stencil
};


AKA_IMPLEMENT_BITMASK_OPERATOR(ClearMask)

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

AKA_IMPLEMENT_BITMASK_OPERATOR(AttachmentFlag)

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

	FramebufferState& addColor(TextureFormat format, AttachmentLoadOp loadOp) {
		AKA_ASSERT(count + 1 < FramebufferMaxColorAttachmentCount, "Too many attachments");
		colors[count++] = Attachment{ format, loadOp };
		return *this;
	}
	FramebufferState& setDepth(TextureFormat format, AttachmentLoadOp loadOp) {
		depth = Attachment{ format, loadOp };
		return *this;
	}
};

class GraphicDevice;

uint32_t getWidth(GraphicDevice* device, const Attachment* colors, uint32_t count, const Attachment* depth);
uint32_t getHeight(GraphicDevice* device, const Attachment* colors, uint32_t count, const Attachment* depth);
FramebufferState getState(GraphicDevice* device, const Attachment* colors, uint32_t count, const Attachment* depth);

struct Framebuffer;
using FramebufferHandle = ResourceHandle<Framebuffer>;

struct Framebuffer : Resource
{
	Framebuffer(const char* name, uint32_t width, uint32_t height, const FramebufferState& state, const Attachment* colors, const Attachment* depth);

	uint32_t width, height;

	FramebufferState framebuffer;

	Attachment colors[FramebufferMaxColorAttachmentCount];
	Attachment depth;

	bool hasDepthStencil() const { return depth.texture != TextureHandle::null; }

	static FramebufferHandle create(const char* name, const Attachment* attachments, uint32_t count, const Attachment* depth);
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
