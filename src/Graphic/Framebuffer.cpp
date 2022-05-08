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

};
};