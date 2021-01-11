#include "GraphicBackend.h"

#include <stdexcept>

namespace app {

uint32_t checkError_(const char* file, int line)
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
        throw std::runtime_error("Error");
    }
    return errorCode;
}

void GraphicBackend::initialize()
{
    m_api = API::OPENGL;
#if !defined(__APPLE__)
	glewExperimental = true; // Nécessaire dans le profil de base
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error("Could not init GLEW");
	}
#endif
}

void GraphicBackend::destroy()
{
	glFinish();
	glBindVertexArray(0);
}

void GraphicBackend::clear(const color4f& color)
{
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GraphicBackend::viewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    m_viewport.x = x;
    m_viewport.y = y;
    m_viewport.width = width;
    m_viewport.height = height;

	glViewport(x, y, width, height);
}

const Viewport& GraphicBackend::viewport() const
{
    return m_viewport;
}

Texture * GraphicBackend::createTexture(uint32_t width, uint32_t height, const uint8_t* data)
{
	// TODO use smart pointer
	Texture *texture = new Texture;
	texture->create(width, height, data);
	return texture;
}

Shader* GraphicBackend::createProgram(const ShaderInfo& info)
{
	Shader* shader = new Shader;
	shader->create(info);
	return shader;
}

ShaderID GraphicBackend::createShader(const char* content, ShaderType type)
{
	return Shader::create(content, type);
}

FontRenderer* GraphicBackend::createFontRenderer()
{
    FontRenderer*font = new FontRenderer;
    font->create();
    return font;
}

}