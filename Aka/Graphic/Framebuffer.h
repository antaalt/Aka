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
		Stencil,
		DepthStencil
	};
	struct Attachment
	{
		AttachmentType type;
		Texture::Ptr texture;
	};
protected:
	Framebuffer(uint32_t width, uint32_t height);
	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;
	virtual ~Framebuffer();
public:

	static Framebuffer::Ptr create(uint32_t width, uint32_t height, Sampler::Filter filter);
	static Framebuffer::Ptr create(uint32_t width, uint32_t height, AttachmentType* attachment, size_t count, Sampler::Filter filter);

	// Get framebuffer width
	uint32_t width() const;

	// Get framebuffer height
	uint32_t height() const;

	// Resize the framebuffer
	virtual void resize(uint32_t width, uint32_t height) = 0;

	// Bind the framebuffer
	virtual void bind(Type type) = 0;

	// Clear the framebuffer
	virtual void clear(float r, float g, float b, float a) = 0;

	// Blit the framebuffer to another one
	virtual void blit(Framebuffer::Ptr dst, const Rect &srcRect, const Rect& dstRect, Sampler::Filter filter) = 0;

protected:
	uint32_t m_width;
	uint32_t m_height;
};

}