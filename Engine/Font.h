#pragma once

#include <freetype/freetype.h>
#include <string>

#include "Geometry.h"

namespace app {

struct Character {
	vec2i size;           // Size of glyph
	vec2i bearing;        // Offset from baseline to left/top of glyph
	uint32_t   textureID; // ID handle of the glyph texture
	uint32_t   advance;   // Offset to advance to next glyph
};

class Font
{
public:
	void create(const char* path);
	virtual void destroy() = 0;

	virtual void render(const std::string& text, float x, float y, float scale, color3f color) = 0;

protected:
	virtual void createBackend(FT_Face face) = 0;

protected:
	Character m_characters[128];
};

}