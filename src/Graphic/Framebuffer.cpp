#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Core/Application.h>
#include <Aka/OS/Logger.h>

namespace aka {

Framebuffer::Framebuffer(uint32_t width, uint32_t height) :
	m_width(width),
	m_height(height),
	m_attachments()
{
}

Framebuffer::Framebuffer(Attachment* attachment, size_t count) :
	m_width(attachment[0].texture->width()),
	m_height(attachment[0].texture->height()),
	m_attachments(attachment, attachment + count)
{
	for (size_t i = 1; i < count; ++i)
	{
		if (m_width > attachment[i].texture->width())
			m_width = attachment[i].texture->width();
		if (m_height > attachment[i].texture->height())
			m_height = attachment[i].texture->height();
	}
}

Framebuffer::~Framebuffer()
{
}

Framebuffer::Ptr Framebuffer::create(Attachment* attachment, size_t count)
{
	// Validate attachment
	for (size_t i = 0; i < count; ++i)
	{
		if (!valid(attachment[i]))
		{
			Logger::error("Attachment not valid : ", i);
			return nullptr;
		}
	}
	return Application::graphic()->createFramebuffer(attachment, count);
}

bool Framebuffer::valid(Attachment attachment)
{
	if (attachment.texture == nullptr)
		return false;
	if ((attachment.texture->flags() & TextureFlag::RenderTarget) != TextureFlag::RenderTarget)
		return false;
	return true;
}

uint32_t Framebuffer::width() const
{
	return m_width;
}

uint32_t Framebuffer::height() const
{
	return m_height;
}

Attachment* Framebuffer::getAttachment(AttachmentType type)
{
	for (Attachment& attachment : m_attachments)
		if (attachment.type == type)
			return &attachment;
	return nullptr;
}

Texture::Ptr Framebuffer::get(AttachmentType type)
{
	for (Attachment& attachment : m_attachments)
	{
		if (attachment.type == type)
			return attachment.texture;
	}
	return nullptr;
}

AttachmentFlag operator&(const AttachmentFlag& lhs, const AttachmentFlag& rhs)
{
	return static_cast<AttachmentFlag>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

AttachmentFlag operator|(const AttachmentFlag& lhs, const AttachmentFlag& rhs)
{
	return static_cast<AttachmentFlag>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

ClearMask operator&(const ClearMask& lhs, const ClearMask& rhs)
{
	return static_cast<ClearMask>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

ClearMask operator|(const ClearMask& lhs, const ClearMask& rhs)
{
	return static_cast<ClearMask>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

};