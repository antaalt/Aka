#include "GLFramebuffer.h"

#include <string>
#include <stdexcept>

namespace app {
namespace gl {

Framebuffer::Framebuffer() : 
	m_status(GL_FRAMEBUFFER_UNDEFINED),
	m_id(0),
	m_renderBuffer(0)
{
}


Framebuffer::~Framebuffer()
{
	if (m_id != 0)
		destroy();
}

void Framebuffer::create(uint32_t width, uint32_t height)
{
	// CHECK EXTENSION WITH GLEW 
	if (!glewIsExtensionSupported("GL_ARB_framebuffer_object"))
		return;

	// Create and attach FBO
	glGenFramebuffers(1, &m_id);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);
	// Create and attach a color buffer
	glGenRenderbuffers(1, &m_renderBuffer);
	// We must bind m_renderBuffer before we call glRenderbufferStorage
	glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
	// The storage format is RGBA8
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_width, m_height);
	// Attach color buffer to FBO
	glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_renderBuffer);

	m_status = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
	if (m_status != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Error while checking frame buffer status : " + std::to_string(m_status));
}

void Framebuffer::destroy()
{
	if (m_renderBuffer != 0)
		glDeleteRenderbuffers(1, &m_renderBuffer);
	if (m_id != 0)
		glDeleteFramebuffers(1, &m_id);
}

void Framebuffer::use()
{
	// Read buffer is created FBO
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);
	// Draw buffer is default FBO, the one used for display
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Framebuffer::doNotUse()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

bool Framebuffer::isValid()
{
	return (m_status == GL_FRAMEBUFFER_COMPLETE);
}

GLuint Framebuffer::getID()
{
	return m_id;
}

}
}