#pragma once
#if defined(AKA_USE_OPENGL)
#include <Aka/Core/Debug.h>
#include <Aka/Platform/Platform.h>

#if defined(AKA_PLATFORM_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#define GLEW_NO_GLU
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Texture2D.h>
#include <Aka/Graphic/Texture2DMultisample.h>
#include <Aka/Graphic/TextureCubeMap.h>
#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Graphic/Backbuffer.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/RenderPass.h>
#include <Aka/Graphic/Device.h>
#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/Program.h>

#define GL_CHECK_RESULT(result)                \
{                                              \
	GLenum res = (result);                     \
	if (GL_NO_ERROR != res) {                  \
		char buffer[256];                      \
		snprintf(                              \
			buffer,                            \
			256,                               \
			"%s (%s at %s:%d)",                \
			glewGetErrorString(res),           \
			AKA_STRINGIFY(result),             \
			__FILE__,                          \
			__LINE__                           \
		);                                     \
		std::cerr << buffer << std::endl;      \
		throw std::runtime_error(res, buffer); \
	}                                          \
}

namespace aka {

class GLFramebuffer;
class GLBackbuffer;

class GLTexture2D;
class GLTextureCubeMap;
class GLTexture2DMultisample;

class GLBuffer;

class GLShader;
class GLProgram;
class GLMaterial;

class GLMesh;


const char* glGetErrorString(GLenum error);

GLuint glType(ShaderType type);

GLenum glType(BufferType type);

GLenum glAccess(BufferUsage usage, BufferCPUAccess access);

GLenum glFilter(TextureFilter type, TextureMipMapMode mode = TextureMipMapMode::None);

GLenum glAttachmentType(AttachmentType type);

GLenum glFormat(IndexFormat format);

GLenum glFormat(VertexFormat format);

GLenum glPrimitive(PrimitiveType type);

GLenum glWrap(TextureWrap wrap);

GLenum glComponentInternal(TextureFormat format);

GLenum glComponent(TextureFormat component);

GLenum glType(TextureType type);

GLenum glFormat(TextureFormat format);

GLenum glBlendMode(BlendMode mode);

GLenum glBlendOp(BlendOp op);

GLenum glStencilCompare(StencilCompare mode);

GLenum glStencilMode(StencilMode op);

};

#endif