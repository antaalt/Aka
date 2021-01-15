#pragma once

#include "Texture.h"
#include "Shader.h"
#include "Framebuffer.h"

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
	enum class API {
		OPENGL,
		DIRECTX
	};
	API api() const { m_api; }
public:

	void initialize();

	void destroy();

	void clear(const color4f& color);

	void viewport(int32_t x, int32_t y, uint32_t width, uint32_t height);

	const Viewport& viewport() const;

private:
	API m_api;
	Viewport m_viewport;
};

}