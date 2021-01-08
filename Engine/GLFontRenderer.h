#pragma once

#include "FontRenderer.h"
#include "GLBackend.h"
#include "GLShader.h"

namespace app {
namespace gl {

class FontRenderer : public app::FontRenderer
{
public:
	void create() override;
	void destroy() override;

	void destroyFont(const Font& font) override;

	void render(const Font &font, const std::string& text, float x, float y, float scale, color3f color) override;
protected:
	Font createFontBackend(FT_Face face) override;

private:
	// TODO this shader is the same for every font, make it static ?
	Shader m_shader; 
	GLuint m_vbo, m_vao;
};

};
};