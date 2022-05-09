#include <Aka/Graphic/Framebuffer.h>

#include <Aka/Core/Application.h>

#include <type_traits>

namespace aka {
namespace gfx {

bool has(ClearMask flags, ClearMask flag) 
{ 
	return (flags & flag) == flag;
}

ClearMask operator&(ClearMask lhs, ClearMask rhs)
{
	return static_cast<ClearMask>(
		static_cast<std::underlying_type<ClearMask>::type>(lhs) &
		static_cast<std::underlying_type<ClearMask>::type>(rhs)
	);
}
ClearMask operator|(ClearMask lhs, ClearMask rhs)
{
	return static_cast<ClearMask>(
		static_cast<std::underlying_type<ClearMask>::type>(lhs) |
		static_cast<std::underlying_type<ClearMask>::type>(rhs)
	);
}
bool has(AttachmentFlag flags, AttachmentFlag flag)
{
	return (flags & flag) == flag;
}

AttachmentFlag operator&(AttachmentFlag lhs, AttachmentFlag rhs)
{
	return static_cast<AttachmentFlag>(
		static_cast<std::underlying_type<AttachmentFlag>::type>(lhs) &
		static_cast<std::underlying_type<AttachmentFlag>::type>(rhs)
	);
}
AttachmentFlag operator|(AttachmentFlag lhs, AttachmentFlag rhs)
{
	return static_cast<AttachmentFlag>(
		static_cast<std::underlying_type<AttachmentFlag>::type>(lhs) |
		static_cast<std::underlying_type<AttachmentFlag>::type>(rhs)
	);
}

FramebufferHandle Framebuffer::create(const Attachment* attachments, uint32_t count, const Attachment* depth)
{
	return Application::app()->graphic()->createFramebuffer(attachments, count, depth);
}
void Framebuffer::destroy(FramebufferHandle framebuffer)
{
	return Application::app()->graphic()->destroy(framebuffer);
}

bool operator<(const FramebufferState& lhs, const FramebufferState& rhs)
{
	if (lhs.count < rhs.count) return true;
	else if (lhs.count > rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.colors[i].format < rhs.colors[i].format) return true;
		else if (lhs.colors[i].format > rhs.colors[i].format) return false;
		if (lhs.colors[i].loadOp < rhs.colors[i].loadOp) return true;
		else if (lhs.colors[i].loadOp > rhs.colors[i].loadOp) return false;
	}
	if (lhs.depth.format < rhs.depth.format) return true;
	else if (lhs.depth.format > rhs.depth.format) return false;
	if (lhs.depth.loadOp < rhs.depth.loadOp) return true;
	else if (lhs.depth.loadOp > rhs.depth.loadOp) return false;
	return false; // equal
}

bool operator>(const FramebufferState& lhs, const FramebufferState& rhs)
{
	if (lhs.count > rhs.count) return true;
	else if (lhs.count < rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.colors[i].format > rhs.colors[i].format) return true;
		else if (lhs.colors[i].format < rhs.colors[i].format) return false;
		if (lhs.colors[i].loadOp > rhs.colors[i].loadOp) return true;
		else if (lhs.colors[i].loadOp < rhs.colors[i].loadOp) return false;
	}
	if (lhs.depth.format > rhs.depth.format) return true;
	else if (lhs.depth.format < rhs.depth.format) return false;
	if (lhs.depth.loadOp > rhs.depth.loadOp) return true;
	else if (lhs.depth.loadOp < rhs.depth.loadOp) return false;
	return false; // equal
}

bool operator==(const FramebufferState& lhs, const FramebufferState& rhs)
{
	if (lhs.count != rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.colors[i].format != rhs.colors[i].format) return false;
		if (lhs.colors[i].loadOp != rhs.colors[i].loadOp) return false;
	}
	if (lhs.depth.format != rhs.depth.format) return false;
	if (lhs.depth.loadOp != rhs.depth.loadOp) return false;
	return true; // equal
}

bool operator!=(const FramebufferState& lhs, const FramebufferState& rhs)
{
	if (lhs.count != rhs.count) return true;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.colors[i].format != rhs.colors[i].format) return true;
		if (lhs.colors[i].loadOp != rhs.colors[i].loadOp) return true;
	}
	if (lhs.depth.format != rhs.depth.format) return true;
	if (lhs.depth.loadOp != rhs.depth.loadOp) return true;
	return false; // equal
}

};
};