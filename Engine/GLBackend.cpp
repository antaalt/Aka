#include "GLBackend.h"

#include <stdexcept>

namespace app {

void GLBackend::initialize()
{
#if !defined(__APPLE__)
	glewExperimental = true; // Nécessaire dans le profil de base
	if (glewInit() != GLEW_OK) {
		glfwTerminate();
		throw std::runtime_error("Could not init GLEW");
	}
#endif
}

void GLBackend::destroy()
{
	glFinish();
	glBindVertexArray(0);
}

void GLBackend::clear()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLBackend::viewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	glViewport(x, y, width, height);
}

Texture * GLBackend::createTexture(uint32_t width, uint32_t height, const uint8_t* data)
{
	// TODO use smart pointer
	gl::Texture *texture = new gl::Texture;
	texture->create(width, height, data);
	return texture;
}

Shader* GLBackend::createProgram(const ShaderInfo& info)
{
	gl::Shader* shader = new gl::Shader;
	shader->create(info);
	return shader;
}

ShaderID GLBackend::createShader(const char* content, ShaderType type)
{
	return gl::Shader::create(content, type);
}

}