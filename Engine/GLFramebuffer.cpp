#include "Framebuffer.h"

#include <string>
#include <stdexcept>

#include "Platform.h"

namespace app {

Framebuffer::Framebuffer() :
	m_width(0),
	m_height(0),
	m_framebufferID(0)
{
}


Framebuffer::~Framebuffer()
{
	if (m_framebufferID != 0)
		destroy();
}

void Framebuffer::create(uint32_t width, uint32_t height, Texture* colorAttachment)
{
	// CHECK EXTENSION WITH GLEW 
	if (!glewIsExtensionSupported("GL_ARB_framebuffer_object"))
		return;

	glGenFramebuffers(1, &m_framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment->getID(), 0);
	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not created");
}

void Framebuffer::destroy()
{
	if (m_framebufferID != 0)
		glDeleteFramebuffers(1, &m_framebufferID);
}

GLenum getType(FramebufferType type) {
	switch (type) {
	case FramebufferType::READ:
		return GL_READ_FRAMEBUFFER;
	case FramebufferType::DRAW:
		return GL_DRAW_FRAMEBUFFER;
	case FramebufferType::BOTH:
		return GL_FRAMEBUFFER;
	}
}

void Framebuffer::bind(FramebufferType type)
{
	glBindFramebuffer(getType(type), m_framebufferID);
}

void Framebuffer::bindDefault(FramebufferType type)
{
	glBindFramebuffer(getType(type), 0);
}

FramebufferID Framebuffer::getID()
{
	return m_framebufferID;
}

};