#include "GLBackend.h"

#include <stdexcept>

namespace app {

void OpenGLBackend::initialize()
{
#if !defined(__APPLE__)
	glewExperimental = true; // Nécessaire dans le profil de base
	if (glewInit() != GLEW_OK) {
		glfwTerminate();
		throw std::runtime_error("Could not init GLEW");
	}
#endif
}

void OpenGLBackend::destroy()
{
	glFinish();
	glBindVertexArray(0);
}

void OpenGLBackend::clear()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

Texture* OpenGLBackend::createTexture()
{
	return nullptr;
}

}