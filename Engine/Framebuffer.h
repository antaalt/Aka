#pragma once

#include <stdint.h>
#include <vector>

#include "Texture.h"
#include "Geometry.h"

namespace aka {

struct Rect {
	float x;
	float y;
	float w;
	float h;
};

class Framebuffer
{
public:
	using Ptr = std::shared_ptr<Framebuffer>;
	using ID = uint32_t;

	enum class Type {
		Read,
		Draw,
		Both
	};
	enum class AttachmentType {
		Color0,
		Color1,
		Color2,
		Color3,
		Depth,
		Stencil
	};
	struct Attachment
	{
		AttachmentType type;
		Texture::Ptr texture;
	};
protected:
	Framebuffer();
public:
	Framebuffer(uint32_t width, uint32_t height, Attachment* attachments, size_t count);
	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;
	virtual ~Framebuffer();

	static Framebuffer::Ptr create(uint32_t width, uint32_t height, Sampler::Filter filter);
	static Framebuffer::Ptr create(uint32_t width, uint32_t height, Attachment*attachment, size_t count);

	// Bind the framebuffer
	void bind(Type type);

	// Get framebuffer width
	virtual uint32_t width() const;

	// Get framebuffer height
	virtual uint32_t height() const;

	// Get underlying ID
	ID id() const;

	// Blit the framebuffer to another one
	void blit(Framebuffer::Ptr dst, const Rect &srcRect, const Rect& dstRect, Sampler::Filter filter);

protected:
	uint32_t m_width;
	uint32_t m_height;
	ID m_framebufferID;
};

}