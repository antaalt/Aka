#pragma once

#include <freetype/freetype.h>
#include <string>

#include "Geometry.h"
#include "FileSystem.h"
#include "Shader.h"

namespace aka {

// TODO use signed distance field font rendering instead
struct Character {
	vec2i size;           // Size of glyph
	vec2i bearing;        // Offset from baseline to left/top of glyph
	uint32_t   textureID; // ID handle of the glyph texture
	uint32_t   advance;   // Offset to advance to next glyph
};

struct Font {
	Character characters[128];

	vec2i size(const std::string& text) const;
};

struct Text {
	Font font;
};

class FontRenderer
{
public:
	void create();
	void destroy();

	Font createFont(const Path &path, uint32_t height);
	void destroyFont(const Font& font);

	void viewport(int32_t x, int32_t y, uint32_t width, uint32_t height);
	void render(const Font& font, const std::string& text, float x, float y, float scale, color3f color);

private:
	Font createFontBackend(FT_Face face);

private:
	Shader m_shader;
	uint32_t m_vbo, m_vao;
	int32_t m_x, m_y;
	uint32_t m_width, m_height;
};

}