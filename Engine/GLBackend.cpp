#include "GLBackend.h"

#include <stdexcept>

#include "GLShader.h"
#include "GLTexture.h"
#include "GLFramebuffer.h"
#include "GLFontRenderer.h"
#include "GLSpriteRenderer.h"

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cerr << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}

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

void GLBackend::clear(const color4f& color)
{
	glClearColor(color.r, color.g, color.b, color.a);
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

FontRenderer* GLBackend::createFontRenderer()
{
    gl::FontRenderer*font = new gl::FontRenderer;
    font->create();
    return font;
}

SpriteRenderer* GLBackend::createSpriteRenderer()
{
    gl::SpriteRenderer* sprite = new gl::SpriteRenderer;
    sprite->create();
    return sprite;
}

}