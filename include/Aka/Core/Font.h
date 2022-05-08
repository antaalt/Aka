#pragma once

#include <Aka/Core/Geometry.h>
#include <Aka/Core/Container/String.h>
#include <Aka/Core/Encoding.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/OS/Path.h>

namespace aka {

struct Character {
	vec2i size;           // Size of glyph
	vec2i bearing;        // Offset from baseline to left/top of glyph
	uint32_t advance;     // Offset to advance to next glyph
	gfx::SubTexture texture;   // Glyph texture
};

class Font
{
public:

	Font(const byte_t* bytes, size_t count, uint32_t height);
	~Font();

	static Font* create(const byte_t* bytes, size_t count, uint32_t height);
	static void destroy(Font* font);

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
	// Get the texture atlas
	gfx::TextureHandle atlas() const;

	// Iterator
	using CharacterIterator = std::vector<Character>::iterator;
	using CharacterConstIterator = std::vector<Character>::const_iterator;
	CharacterIterator begin();
	CharacterConstIterator begin() const;
	CharacterIterator end();
	CharacterConstIterator end() const;
private:
	uint32_t m_height;
	uint32_t m_advance;
	String m_familyName;
	String m_styleName;
	gfx::TextureHandle m_atlas;
	std::vector<Character> m_characters;
};


};
