#pragma once

#include "Geometry.h"
#include "../Graphic/Texture.h"
#include "../OS/FileSystem.h"

#define NUM_GLYPH 255

namespace aka {

// TODO use signed distance field font rendering instead
struct Character {
	vec2i size;           // Size of glyph
	vec2i bearing;        // Offset from baseline to left/top of glyph
	uint32_t advance;     // Offset to advance to next glyph
	SubTexture texture;   // Glyph texture
};

struct Font {
	Font(const Path& path, uint32_t height);

	
	static Font create(const Path& path, uint32_t height);


	vec2i size(const std::string& text) const;

	const Character &character(char c) const;

	const std::string& family() const;

	const std::string& style() const;

	uint32_t height() const;

private:
	uint32_t m_height;
	std::string m_familyName;
	std::string m_styleName;
	Character m_characters[NUM_GLYPH];
};


};
