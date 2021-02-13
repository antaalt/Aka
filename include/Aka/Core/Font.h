#pragma once

#include "Geometry.h"
#include "../Graphic/Texture.h"
#include "../OS/FileSystem.h"

namespace aka {

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

	const Character &getCharacter(uint32_t c) const;

	size_t count() const;

	const std::string& family() const;

	const std::string& style() const;

	uint32_t height() const;

	uint32_t advance() const;

private:
	uint32_t m_height;
	uint32_t m_advance;
	std::string m_familyName;
	std::string m_styleName;
	std::vector<Character> m_characters;
};


};
