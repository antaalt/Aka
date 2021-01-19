#include "../GraphicBackend.h"

#include "GLBackend.h"
#include "../../Platform/Logger.h"

#include <stdexcept>

void APIENTRY openglCallbackFunction(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam
) {
    std::string errorType;
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        errorType = "error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        errorType = "deprecated_behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        errorType = "undefined_behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        errorType = "portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        errorType = "performance";
        break;
    case GL_DEBUG_TYPE_OTHER:
        errorType = "other";
        break;
    }
    switch (severity) {
    case GL_DEBUG_SEVERITY_LOW:
        aka::Logger::debug("[", errorType, "][low] ", message);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        aka::Logger::warn("[", errorType, "][medium] ", message);
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        aka::Logger::error("[", errorType, "][high] ", message);
        break;
    }
}

namespace aka {

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
    m_api = Api::OpenGL;
#if !defined(__APPLE__)
	glewExperimental = true; // Nécessaire dans le profil de base
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error("Could not init GLEW");
	}
#endif

#if _DEBUG
    if (glDebugMessageCallback) {
        Logger::info("Setting up openGL callback.");
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openglCallbackFunction, nullptr);
        GLuint unused = 0;
        glDebugMessageControl(
            GL_DONT_CARE,
            GL_DONT_CARE,
            GL_DONT_CARE,
            0,
            &unused,
            GL_TRUE
        );
    }
    else
        Logger::warn("glDebugMessageCallback not supported");
#endif
    m_backbuffer = Backbuffer::create();
}

void GraphicBackend::destroy()
{
	glFinish();
	glBindVertexArray(0);
}

void GraphicBackend::resize(uint32_t width, uint32_t height)
{
    m_backbuffer->setSize(width, height);
}

void GraphicBackend::clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GraphicBackend::viewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	glViewport(x, y, width, height);
}

Rect GraphicBackend::viewport() const
{
    GLint dims[4] = { 0 };
    glGetIntegerv(GL_VIEWPORT, dims);
    return Rect{ (float)dims[0], (float)dims[1], (float)dims[2], (float)dims[3] };
}

Backbuffer::Ptr GraphicBackend::backbuffer() const
{
    return m_backbuffer;
}

}