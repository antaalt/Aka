#pragma once

#include "Geometry.h"
#include "Texture.h"
#include "FileSystem.h"

namespace aka {

// TODO use signed distance field font rendering instead
struct Character {
	vec2i size;           // Size of glyph
	vec2i bearing;        // Offset from baseline to left/top of glyph
	uint32_t advance;     // Offset to advance to next glyph
	Texture::Ptr texture; // Glyph texture
};

struct Font {
	Character characters[128];

	static Font create(const Path& path, uint32_t height);

	vec2i size(const std::string& text) const;
};


};
