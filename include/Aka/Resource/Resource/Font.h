#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Resource.h>

namespace aka {

struct FontRenderData : RenderData
{
	gfx::TextureHandle texture;
};

struct FontBuildData : BuildData
{
	uint32_t height;
	Blob ttf;
};

struct Character {
	vec2i size;           // Size of glyph
	vec2i bearing;        // Offset from baseline to left/top of glyph
	uint32_t advance;     // Offset to advance to next glyph
	gfx::SubTexture texture;   // Glyph texture
};

class Font : public Resource
{
public:
	Font();
	~Font();

	void createBuildData() override;
	void createBuildData(gfx::GraphicDevice* device, RenderData* data) override;
	void createRenderData(gfx::GraphicDevice* device, const BuildData* data) override;
	void destroyRenderData(gfx::GraphicDevice* device) override;
	ResourceArchive* createResourceArchive() override;


	// Get the size of the given string depending on the font
	vec2i size(const String& text) const;
	// Get the character information for a specific codepoint
	const Character& getCharacter(encoding::CodePoint c) const;
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