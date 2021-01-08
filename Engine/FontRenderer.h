#pragma once

#include <freetype/freetype.h>
#include <string>

#include "Geometry.h"
#include "System.h"

namespace app {

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

	virtual void create() = 0;
	virtual void destroy() = 0;

	Font createFont(const Path &path, uint32_t height);
	virtual void destroyFont(const Font& font) = 0;

	void viewport(int32_t x, int32_t y, uint32_t width, uint32_t height);
	virtual void render(const Font& font, const std::string& text, float x, float y, float scale, color3f color) = 0;

protected:
	virtual Font createFontBackend(FT_Face face) = 0;

protected:
	int32_t m_x, m_y;
	uint32_t m_width, m_height;
};

}