#pragma once

#include <stdint.h>
#include <vector>

#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/TextureCubeMap.h>
#include <Aka/Core/Geometry.h>

namespace aka {

enum class ClearMask
{
	None = 0,
	Color = 1,
	Depth = 2,
	Stencil = 4,
	All = Color | Depth | Stencil
};

enum class AttachmentType
{
	Color0,
	Color1,
	Color2,
	Color3,
	Depth,
	Stencil,
	DepthStencil
};

enum class AttachmentFlag
{
	None = 0,
	AttachTextureObject = (1 << 0), // Attach the object instead of the layer
};

struct Attachment
{
	AttachmentType type; // Type of the attachment
	Texture::Ptr texture; // Texture used as attachment
	AttachmentFlag flag; // Attachment flag
	uint32_t layer; // Layer of the texture used as attachment (if AttachmentFlag::AttachTextureLayer set)
	uint32_t level; // Level of the mips used as attachment
};

class Framebuffer
{
public:
	using Ptr = std::shared_ptr<Framebuffer>;

protected:
	Framebuffer(uint32_t width, uint32_t height);
	Framebuffer(Attachment* attachment, size_t count);
	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;
	virtual ~Framebuffer();
public:
	// Create a framebuffer from attachment
	static Framebuffer::Ptr create(Attachment* attachment, size_t count);
	// Check if an attachment is valid
	static bool valid(Attachment attachment);

	// Get framebuffer width
	uint32_t width() const;
	// Get framebuffer height
	uint32_t height() const;

	// Clear the framebuffer
	virtual void clear(const color4f& color, float depth = 1.f, int stencil = 1, ClearMask mask = ClearMask::All) = 0;

	// Get the framebuffer attachment
	Attachment* getAttachment(AttachmentType type);
	// Get the texture of the framebuffer attachment
	Texture::Ptr get(AttachmentType type);
	// Set the attachment of the framebuffer
	virtual void set(AttachmentType type, Texture::Ptr texture, AttachmentFlag flag = AttachmentFlag::None, uint32_t layer = 0, uint32_t level = 0) = 0;

protected:
	uint32_t m_width;
	uint32_t m_height;
	std::vector<Attachment> m_attachments;
};

AttachmentFlag operator&(const AttachmentFlag& lhs, const AttachmentFlag& rhs);
AttachmentFlag operator|(const AttachmentFlag& lhs, const AttachmentFlag& rhs);

ClearMask operator&(const ClearMask& lhs, const ClearMask& rhs);
ClearMask operator|(const ClearMask& lhs, const ClearMask& rhs);

}