#include "Font.h"

#include <freetype/freetype.h>

#include "Logger.h"

namespace aka {

Font Font::create(const Path& path, uint32_t height)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        throw std::runtime_error("ERROR::FREETYPE: Could not init FreeType Library");

    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face))
        throw std::runtime_error("ERROR::FREETYPE: Failed to load font");

    FT_Set_Pixel_Sizes(face, 0, height);

    Font font;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            Logger::error("[freetype] Failed to load glyph '", (char)c,"'");
            continue;
        }
        // now store character for later use
        font.characters[c] = {
            vec2i(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            vec2i(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<uint32_t>(face->glyph->advance.x),
            Texture::create(face->glyph->bitmap.width, face->glyph->bitmap.rows, Texture::Format::Red, Texture::Format::Red, face->glyph->bitmap.buffer, Sampler::Filter::Nearest)
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
    return font;
}

vec2i Font::size(const std::string& text) const
{
    vec2i size(0);
    for (const char& c : text)
    {
        const Character& ch = characters[c];
        size.x += (ch.advance >> 6); // bitshift by 6 to get value in pixels (2^6 = 64)
        size.y = max(size.y, ch.size.y);
    }
    return size;
}

}