#pragma once

#include "Geometry.h"
#include "../Graphic/Texture.h"
#include "../Platform/IO/FileSystem.h"

namespace aka {

// TODO use signed distance field font rendering instead
struct Character {
	vec2i size;           // Size of glyph
	vec2i bearing;        // Offset from baseline to left/top of glyph
	uint32_t advance;     // Offset to advance to next glyph
	Texture::Ptr texture; // Glyph texture
};

struct Font {
	Font(const Path& path, uint32_t height);

	
	static Font create(const Path& path, uint32_t height);


	vec2i size(const std::string& text) const;

	const Character &character(char c) const;

	const std::string& family() const;

	const std::string style() const;

private:
	std::string m_familyName;
	std::string m_styleName;
	Character m_characters[128];
};


};