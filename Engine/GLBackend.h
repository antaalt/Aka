#pragma once

#include "GraphicBackend.h"
#include "Platform.h"

#include <iostream>

GLenum glCheckError_(const char* file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

namespace app {

struct GLBackend : public GraphicBackend {
public:
	void initialize() override;
	void destroy() override;

	void clear() override;

	void viewport(int32_t x, int32_t y, uint32_t width, uint32_t height) override;

	Texture *createTexture(uint32_t width, uint32_t height, const uint8_t *data) override;
	Shader* createProgram(const ShaderInfo& info) override;
	ShaderID createShader(const char* content, ShaderType type) override;
	Font* createFont(const char* path) override;
private:
	API m_api;
};

}
