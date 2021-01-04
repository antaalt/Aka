#pragma once

#include "GLBackend.h"

namespace app {
namespace gl {

class Framebuffer : app::Framebuffer
{
public:
	Framebuffer();
	~Framebuffer();

	void create(uint32_t width, uint32_t height) override;
	void destroy() override;

	void use() override;
	void doNotUse();

	bool isValid();

	GLuint getID();

private:
	GLuint m_renderBuffer;		// RenderBuffer linked to the FBO
	GLuint m_id;				// ID of the FBO
	GLenum m_status;			// status of the FBO
};

}
}