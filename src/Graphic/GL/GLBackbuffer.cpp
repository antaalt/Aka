#if defined(AKA_USE_OPENGL)
#include "GLBackbuffer.h"

#include "GLContext.h"

#include <Aka/OS/Logger.h>

namespace aka {

GLBackbuffer::GLBackbuffer(GLFWwindow* window, uint32_t width, uint32_t height) :
	Backbuffer(width, height),
	m_window(window),
	m_vsync(true)
{
}

GLBackbuffer::~GLBackbuffer()
{
}

void GLBackbuffer::clear(const color4f& color, float depth, int stencil, ClearMask mask)
{
	if (mask == ClearMask::None)
		return;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	GLenum glMask = 0;
	if ((mask & ClearMask::Color) == ClearMask::Color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
		glMask |= GL_COLOR_BUFFER_BIT;
	}
	if ((mask & ClearMask::Depth) == ClearMask::Depth)
	{
		glDepthMask(true);
		glClearDepth(depth);
		glMask |= GL_DEPTH_BUFFER_BIT;
	}
	if ((mask & ClearMask::Stencil) == ClearMask::Stencil)
	{
		glClearStencil(stencil);
		glMask |= GL_STENCIL_BUFFER_BIT;
	}
	glClear(glMask);
}

void GLBackbuffer::resize(uint32_t width, uint32_t height)
{
}

void GLBackbuffer::set(AttachmentType type, Texture::Ptr texture, AttachmentFlag flag, uint32_t layer, uint32_t level)
{
	Logger::error("Trying to set backbuffer attachement.");
}


void GLBackbuffer::set(Synchronisation sync)
{
	glfwSwapInterval((int)sync);
}

void GLBackbuffer::blit(const Texture::Ptr& texture, TextureFilter filter)
{
	GLuint blitFBO = 0;
	GLenum attachment = GL_COLOR_ATTACHMENT0;
	GLenum mask = GL_COLOR_BUFFER_BIT;
	if (isDepthStencil(texture->format()))
	{
		attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		mask = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
	}
	else if (isDepth(texture->format()))
	{
		attachment = GL_DEPTH_ATTACHMENT;
		mask = GL_DEPTH_BUFFER_BIT;
	}
	glGenFramebuffers(1, &blitFBO);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, blitFBO); // src
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, attachment, GL_TEXTURE_2D, (GLuint)texture->handle().value(), 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // dst
	glBlitFramebuffer(
		0, 0, texture->width(), texture->height(),
		0, 0, m_width, m_height,
		mask, glFilter(filter)
	);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &blitFBO);
}

void GLBackbuffer::frame()
{
#if defined(AKA_DEBUG)
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
		Logger::error("[GL] Error during frame : ", glGetErrorString(errorCode));
#endif
}

void GLBackbuffer::submit()
{
	glfwSwapBuffers(m_window);
}

void GLBackbuffer::download(void* data)
{
	glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

};

#endif