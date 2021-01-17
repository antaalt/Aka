#pragma once

#include "Texture.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "Backbuffer.h"

namespace aka {

uint32_t checkError_(const char* file, int line);
#define checkError() checkError_(__FILE__, __LINE__) 

struct Viewport {
	int32_t x;
	int32_t y;
	uint32_t width;
	uint32_t height;
};

class GraphicBackend {
public:
	enum class Api {
		OpenGL,
		DirectX
	};
	Api api() const { m_api; }
public:
	void initialize();
	void destroy();

	void resize(uint32_t width, uint32_t height);
public:

	void clear(const color4f& color);

	void viewport(int32_t x, int32_t y, uint32_t width, uint32_t height);

	Rect viewport() const;

	Backbuffer::Ptr backbuffer() const;

protected:

private:
	Backbuffer::Ptr m_backbuffer;
	Api m_api;
};

}