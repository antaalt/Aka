#pragma once

#include "Font.h"
#include "GLBackend.h"
#include "GLShader.h"

namespace app {
namespace gl {

class Font : public app::Font
{
public:
	void destroy() override;

	void render(const std::string& text, float x, float y, float scale, color3f color) override;
protected:
	void createBackend(FT_Face face) override;

private:
	// TODO this shader is the same for every font, make it static ?
	Shader m_shader; 
	GLuint m_vbo, m_vao;
};

};
};