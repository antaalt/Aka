#pragma once

#include "GLBackend.h"

namespace app {
namespace gl {

class Framebuffer
{
public:
	Framebuffer();
	Framebuffer(uint32_t width, uint32_t height);
	~Framebuffer();

	void use();

	void doNotUse();

	bool isValid();

	GLuint getID();

private:
	uint32_t m_width;
	uint32_t m_height;
	GLuint m_renderBuffer;		// RenderBuffer linked to the FBO
	GLuint m_id;				// ID of the FBO
	GLenum m_status;			// status of the FBO
};

}
}