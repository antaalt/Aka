#pragma once

#include "Texture.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "FontRenderer.h"
#include "SpriteRenderer.h"

namespace app {

struct GraphicBackend {
	enum class API {
		OPENGL,
		DIRECTX
	};
	struct Config {
		API api;
	};

	API api() const { m_api; }

public:
	virtual void initialize() = 0;
	virtual void destroy() = 0;

	virtual void clear(const color4f& color) = 0;

	virtual void viewport(int32_t x, int32_t y, uint32_t width, uint32_t height) = 0;

	virtual Texture *createTexture(uint32_t width, uint32_t height, const uint8_t* data) = 0;
	virtual Shader * createProgram(const ShaderInfo& info) = 0;
	virtual ShaderID createShader(const char* shader, ShaderType type) = 0;
	virtual FontRenderer* createFont(const char* path) = 0;
	virtual SpriteRenderer* createSprite() = 0;
private:
	API m_api;
};

}