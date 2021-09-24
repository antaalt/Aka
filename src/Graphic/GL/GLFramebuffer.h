#if defined(AKA_USE_OPENGL)
#pragma once

#include "GLContext.h"

#include <Aka/Graphic/Framebuffer.h>

namespace aka {

class GLFramebuffer : public Framebuffer
{
	friend class GLDevice;
public:
	GLFramebuffer(Attachment* attachments, size_t count);
	~GLFramebuffer();
	void clear(const color4f& color, float depth, int stencil, ClearMask mask) override;
	void set(AttachmentType type, Texture::Ptr texture, AttachmentFlag flag, uint32_t layer, uint32_t level) override;
	void attach(const Attachment& attachment);
	void computeSize();
private:
	GLuint m_framebufferID;
};

};

#endif