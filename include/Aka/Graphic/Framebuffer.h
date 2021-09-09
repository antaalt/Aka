#pragma once

#include <stdint.h>
#include <vector>

#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/TextureCubeMap.h>
#include <Aka/Core/Geometry.h>

namespace aka {

enum class ClearMask {
	None = 0,
	Color = 1,
	Depth = 2,
	Stencil = 4,
	All = Color | Depth | Stencil
};

enum class FramebufferAttachmentType {
	Color0,
	Color1,
	Color2,
	Color3,
	Depth,
	Stencil,
	DepthStencil
};

struct FramebufferAttachment
{
	FramebufferAttachmentType type;
	Texture::Ptr texture;
};

class Framebuffer
{
public:
	using Ptr = std::shared_ptr<Framebuffer>;

protected:
	Framebuffer(uint32_t width, uint32_t height);
	Framebuffer(FramebufferAttachment* attachment, size_t count);
	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;
	virtual ~Framebuffer();
public:
	static Framebuffer::Ptr create(FramebufferAttachment* attachment, size_t count);

	// Get framebuffer width
	uint32_t width() const;
	// Get framebuffer height
	uint32_t height() const;

	// Clear the framebuffer
	virtual void clear(const color4f& color, float depth = 1.f, int stencil = 1, ClearMask mask = ClearMask::All) = 0;
	// Blit a whole framebuffer into another one
	void blit(Framebuffer::Ptr src, FramebufferAttachmentType type, TextureFilter filter);
	// Blit a framebuffer region into another one
	virtual void blit(Framebuffer::Ptr src, Rect rectSrc, Rect rectDst, FramebufferAttachmentType type, TextureFilter filter) = 0;
	// Get the attachment of the framebuffer
	Texture::Ptr get(FramebufferAttachmentType type);
	// Set the attachment of the framebuffer
	virtual void set(FramebufferAttachmentType type, Texture::Ptr texture) = 0;
	// Set the attachment of the framebuffer for a single face of a cubemap
	virtual void set(FramebufferAttachmentType type, TextureCubeMap::Ptr texture, TextureCubeFace face) = 0;

protected:
	uint32_t m_width;
	uint32_t m_height;
	std::vector<FramebufferAttachment> m_attachments;
};

ClearMask operator&(const ClearMask& lhs, const ClearMask& rhs);
ClearMask operator|(const ClearMask& lhs, const ClearMask& rhs);

}