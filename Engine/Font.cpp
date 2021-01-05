#include "Font.h"

#include <stdexcept>

#include "Shader.h"

namespace app {

void Font::create(const char* path)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        throw std::runtime_error("ERROR::FREETYPE: Could not init FreeType Library");

    FT_Face face;
    if (FT_New_Face(ft, path, 0, &face))
        throw std::runtime_error("ERROR::FREETYPE: Failed to load font");

    FT_Set_Pixel_Sizes(face, 0, 48);

    createBackend(face);

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

}