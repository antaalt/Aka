#pragma once

#define GLEW_NO_GLU
#include <gl/glew.h>
#include <gl/gl.h>

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
			STRINGIFY(result),                 \
			__FILE__,                          \
			__LINE__                           \
		);                                     \
		std::cerr << buffer << std::endl;      \
		throw std::runtime_error(res, buffer); \
	}                                          \
}


#include "Framebuffer.h"
#include "Texture.h"
#include "Logger.h"

namespace aka {
namespace gl {

inline GLenum attachmentType(Framebuffer::AttachmentType type)
{
	switch (type)
	{
	default:
		Logger::warn("Framebuffer attachment type not defined : ", (int)type);
		return GL_COLOR_ATTACHMENT0;
	case Framebuffer::AttachmentType::Color0:
		return GL_COLOR_ATTACHMENT0;
	case Framebuffer::AttachmentType::Color1:
		return GL_COLOR_ATTACHMENT1;
	case Framebuffer::AttachmentType::Color2:
		return GL_COLOR_ATTACHMENT2;
	case Framebuffer::AttachmentType::Color3:
		return GL_COLOR_ATTACHMENT3;
	case Framebuffer::AttachmentType::Depth:
		return GL_DEPTH_ATTACHMENT;
	case Framebuffer::AttachmentType::Stencil:
		return GL_STENCIL_ATTACHMENT;
	}
}

inline GLenum framebufferType(Framebuffer::Type type) {
	switch (type) {
	case Framebuffer::Type::Read:
		return GL_READ_FRAMEBUFFER;
	case Framebuffer::Type::Draw:
		return GL_DRAW_FRAMEBUFFER;
	default:
	case Framebuffer::Type::Both:
		return GL_FRAMEBUFFER;
	}
}
inline GLenum filter(Sampler::Filter type) {
	switch (type) {
	default:
	case Sampler::Filter::Linear:
		return GL_LINEAR;
	case Sampler::Filter::Nearest:
		return GL_NEAREST;
	}
}

inline GLenum format(Texture::Format format) {
	switch (format) {
	default:
		throw std::runtime_error("Not implemneted");
	case Texture::Format::Red:
		return GL_RED;
	case Texture::Format::Rgba:
		return GL_RGBA;
	case Texture::Format::Rgba8:
		return GL_RGBA8;
	}
}

};
};