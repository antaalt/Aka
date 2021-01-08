#include "FontRenderer.h"

#include <stdexcept>

#include "Shader.h"

namespace app {

void FontRenderer::create()
{
}

Font FontRenderer::createFont(const Path& path, uint32_t height)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        throw std::runtime_error("ERROR::FREETYPE: Could not init FreeType Library");

    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face))
        throw std::runtime_error("ERROR::FREETYPE: Failed to load font");

    FT_Set_Pixel_Sizes(face, 0, height);

    Font font = createFontBackend(face);

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

void FontRenderer::viewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
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