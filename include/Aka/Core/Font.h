#pragma once

#include <Aka/Core/Geometry.h>
#include <Aka/Core/String.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/OS/FileSystem.h>

namespace aka {

struct Character {
	vec2i size;           // Size of glyph
	vec2i bearing;        // Offset from baseline to left/top of glyph
	uint32_t advance;     // Offset to advance to next glyph
	SubTexture texture;   // Glyph texture
};

class Font
{
public:
	Font(const Path& path, uint32_t height);

	// Get the size of the given string depending on the font
	vec2i size(const String& text) const;
	// Get the character information for a specific codepoint
	const Character &getCharacter(encoding::CodePoint c) const;
	// Get the number of characters
	size_t count() const;
	// Get the family of the font
	const String& family() const;
	// Get the style of the font
	const String& style() const;
	// Get the height of the font
	uint32_t height() const;
	// Get the advance of the font
	uint32_t advance() const;
private:
	uint32_t m_height;
	uint32_t m_advance;
	String m_familyName;
	String m_styleName;
	std::vector<Character> m_characters;
};


};
