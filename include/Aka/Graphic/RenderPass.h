#pragma once

#include <stdint.h>

#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Resource.h>

namespace aka {
namespace gfx {

static constexpr uint32_t FramebufferMaxColorAttachmentCount = 8;

enum class ClearMask : uint8_t
{
	None		= 0,

	Color		= 1 << 0,
	Depth		= 1 << 1,
	Stencil		= 1 << 2,

	All = Color | Depth | Stencil
};
AKA_IMPLEMENT_BITMASK_OPERATOR(ClearMask)

enum class AttachmentLoadOp : uint8_t
{
	Clear,
	Load,
	DontCare,
};
enum class AttachmentStoreOp : uint8_t
{
	Store,
	DontCare,
};

// TODO should be just RenderPass.
struct RenderPassState
{
	struct Attachment
	{
		TextureFormat format;
		AttachmentLoadOp loadOp;
		AttachmentStoreOp storeOp;
		ResourceAccessType initialLayout; // Needed if loadOp = load
		ResourceAccessType finalLayout;
	};

	uint32_t count; // color attachment count
	Attachment colors[FramebufferMaxColorAttachmentCount];
	Attachment depth;

	bool hasDepthStencil() const { return depth.format != TextureFormat::Unknown; }

	RenderPassState& addColor(TextureFormat format, AttachmentLoadOp loadOp = AttachmentLoadOp::Clear, AttachmentStoreOp storeOp = AttachmentStoreOp::Store, ResourceAccessType initialLayout = ResourceAccessType::Attachment, ResourceAccessType finalLayout = ResourceAccessType::Attachment)
	{
		AKA_ASSERT(count + 1 < FramebufferMaxColorAttachmentCount, "Too many attachments");
		colors[count++] = Attachment{ format, loadOp, storeOp, initialLayout, finalLayout };
		return *this;
	}
	RenderPassState& setDepth(TextureFormat format, AttachmentLoadOp loadOp = AttachmentLoadOp::Clear, AttachmentStoreOp storeOp = AttachmentStoreOp::Store, ResourceAccessType initialLayout = ResourceAccessType::Attachment, ResourceAccessType finalLayout = ResourceAccessType::Attachment)
	{
		depth = Attachment{ format, loadOp, storeOp, initialLayout, finalLayout };
		return *this;
	}
};

class GraphicDevice;

struct RenderPass;
using RenderPassHandle = ResourceHandle<RenderPass>;

struct RenderPass : Resource
{
	RenderPass(const char* name, const RenderPassState& state);

	RenderPassState state;
};

bool operator<(const RenderPassState& lhs, const RenderPassState& rhs);
bool operator>(const RenderPassState& lhs, const RenderPassState& rhs);
bool operator==(const RenderPassState& lhs, const RenderPassState& rhs);
bool operator!=(const RenderPassState& lhs, const RenderPassState& rhs);

};
};

template <>
struct std::hash<aka::gfx::RenderPassState>
{
	size_t operator()(const aka::gfx::RenderPassState& data) const
	{
		size_t hash = 0;
		aka::hash::combine(hash, data.count);
		for (size_t i = 0; i < data.count; i++)
		{
			aka::hash::combine(hash, static_cast<uint32_t>(data.colors[i].format));
			aka::hash::combine(hash, static_cast<uint32_t>(data.colors[i].loadOp));
			aka::hash::combine(hash, static_cast<uint32_t>(data.colors[i].storeOp));
			aka::hash::combine(hash, static_cast<uint32_t>(data.colors[i].initialLayout));
			aka::hash::combine(hash, static_cast<uint32_t>(data.colors[i].finalLayout));
		}
		aka::hash::combine(hash, static_cast<uint32_t>(data.depth.format));
		aka::hash::combine(hash, static_cast<uint32_t>(data.depth.loadOp));
		aka::hash::combine(hash, static_cast<uint32_t>(data.depth.storeOp));
		aka::hash::combine(hash, static_cast<uint32_t>(data.depth.initialLayout));
		aka::hash::combine(hash, static_cast<uint32_t>(data.depth.finalLayout));
		return hash;
	}
};
