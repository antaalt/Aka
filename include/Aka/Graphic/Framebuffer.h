#pragma once

#include <stdint.h>
#include <vector>

#include <Aka/Graphic/Texture.h>
#include <Aka/Core/Geometry.h>

namespace aka {

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

	static Framebuffer::Ptr create(uint32_t width, uint32_t height, Sampler sampler);
	static Framebuffer::Ptr create(uint32_t width, uint32_t height, AttachmentType* attachment, size_t count, Sampler sampler);

	// Get framebuffer width
	uint32_t width() const;

	// Get framebuffer height
	uint32_t height() const;

	// Resize the framebuffer
	virtual void resize(uint32_t width, uint32_t height) = 0;

	// Clear the framebuffer
	virtual void clear(float r, float g, float b, float a) = 0;

	// Get the attachment of the framebuffer
	virtual Texture::Ptr attachment(AttachmentType type) = 0;

protected:
	uint32_t m_width;
	uint32_t m_height;
};

}