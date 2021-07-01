#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/OS/Logger.h>

namespace aka {

Framebuffer::Framebuffer(uint32_t width, uint32_t height, FramebufferAttachment* attachment, size_t count) :
	m_width(width),
	m_height(height),
	m_attachments(attachment, attachment + count)
{
}

Framebuffer::~Framebuffer()
{
}

Framebuffer::Ptr Framebuffer::create(uint32_t width, uint32_t height)
{
	FramebufferAttachment colorAttachment;
	colorAttachment.type = FramebufferAttachmentType::Color0;
	colorAttachment.texture = Texture::create2D(width, height, TextureFormat::UnsignedByte, TextureComponent::RGBA, TextureFlag::RenderTarget, Sampler{});
	return create(width, height, &colorAttachment, 1);
}

Framebuffer::Ptr Framebuffer::create(uint32_t width, uint32_t height, FramebufferAttachment* attachment, size_t count)
{
	// Validate attachment
	for (size_t i = 0; i < count; ++i)
	{
		if (attachment[i].texture == nullptr)
		{
			Logger::error("No texture set for framebuffer attachment ", i);
			return nullptr;
		}
		if (attachment[i].texture->width() != width || attachment[i].texture->height() != height)
		{
			Logger::error("Invalid texture size for framebuffer attachment ", i);
			return nullptr;
		}
		if ((attachment[i].texture->flags() & TextureFlag::RenderTarget) != TextureFlag::RenderTarget)
		{
			Logger::error("Invalid flag for framebuffer attachment ", i);
			return nullptr;
		}
	}
	return GraphicBackend::createFramebuffer(width, height, attachment, count);
}

uint32_t Framebuffer::width() const
{
	return m_width;
}

uint32_t Framebuffer::height() const
{
	return m_height;
}
void Framebuffer::blit(Framebuffer::Ptr src, FramebufferAttachmentType type, Sampler::Filter filter)
{
	blit(
		src,
		Rect{ 0,0, src->width(), src->height() },
		Rect{ 0,0, this->width(), this->height() },
		type,
		filter
	);
}

Texture::Ptr Framebuffer::attachment(FramebufferAttachmentType type)
{
	for (FramebufferAttachment& attachment : m_attachments)
	{
		if (attachment.type == type)
			return attachment.texture;
	}
	return nullptr;
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