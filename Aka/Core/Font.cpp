#include "Font.h"

#include <freetype/freetype.h>

#include "../Platform/Logger.h"

namespace aka {

Font::Font(const Path& path, uint32_t height)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        throw std::runtime_error("ERROR::FREETYPE: Could not init FreeType Library");

    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face))
        throw std::runtime_error("ERROR::FREETYPE: Failed to load font");
    FT_Set_Pixel_Sizes(face, 0, height);

    m_familyName = face->family_name;
    m_styleName = face->style_name;

    // TODO build atlas texture for font packer
    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            Logger::error("[freetype] Failed to load glyph '", (char)c, "'");
            continue;
        }
        uint32_t characterSize = face->glyph->bitmap.width * face->glyph->bitmap.rows;
        std::vector<uint8_t> rgba(characterSize * 4);
        for (uint32_t iSrc = 0, iDst = 0; iSrc < characterSize; iSrc++, iDst += 4)
        {
            rgba[iDst + 0] = face->glyph->bitmap.buffer[iSrc];
            rgba[iDst + 1] = face->glyph->bitmap.buffer[iSrc];
            rgba[iDst + 2] = face->glyph->bitmap.buffer[iSrc];
            rgba[iDst + 3] = face->glyph->bitmap.buffer[iSrc];
        }
        // now store character for later use
        m_characters[c] = {
            vec2i(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            vec2i(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<uint32_t>(face->glyph->advance.x),
            Texture::create(face->glyph->bitmap.width, face->glyph->bitmap.rows, Texture::Format::Rgba8, Texture::Format::Rgba, rgba.data(), Sampler::Filter::Nearest)
        };
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // TODO use stb_truetype.h instead ?

    /*stbtt_fontinfo font;
    std::vector<uint8_t> data = loadFromBinaryFile(path);
    stbtt_InitFont(&font, data.data(), 0);

    int ascent;
    int descent;
    int line_gap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);*/
}

Font Font::create(const Path& path, uint32_t height)
{
    return Font(path, height);
}

vec2i Font::size(const std::string& text) const
{
    vec2i size(0);
    for (const char& c : text)
    {
        const Character& ch = m_characters[c];
        size.x += (ch.advance >> 6); // bitshift by 6 to get value in pixels (2^6 = 64)
        size.y = max(size.y, ch.size.y);
    }
    return size;
}

const Character& Font::character(char c) const
{
    return m_characters[c];
}

const std::string& Font::family() const
{
    return m_familyName;
}

const std::string Font::style() const
{
    return m_styleName;
}

}