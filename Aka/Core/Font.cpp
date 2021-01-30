#include "Font.h"

#include <freetype/freetype.h>
#include <utf8.h>

#include "../OS/Logger.h"
#include "../Core/Debug.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define FREETYPE_CHECK_RESULT(result)\
{                                    \
    FT_Error res = (result);         \
    if (res) {                       \
        char buffer[256];            \
        snprintf(                    \
            buffer,                  \
            256,                     \
            "%s (%s at %s:%d)",      \
            "[freetype]",            \
            STRINGIFY(result),       \
            __FILE__,                \
            __LINE__				 \
        );							 \
        ::aka::Logger::error(buffer);\
        throw std::runtime_error(buffer);\
	}								 \
}

namespace aka {

// Basic packer to create font atlas
// TODO optimize
// https://en.wikipedia.org/wiki/Bin_packing_problem
struct Packer {
    Packer(uint32_t elements, uint32_t elementWidth, uint32_t elementHeight) :
        m_elements(elements),
        m_elementCount((uint32_t)ceil<float>(sqrt<float>((float)elements)), (uint32_t)floor<float>(sqrt<float>((float)elements) + 0.5f)),
        m_elementSize(elementWidth, elementHeight),
        m_atlasSize(m_elementCount * m_elementSize),
        m_data(m_atlasSize.x * m_atlasSize.y * 4),
        m_subTexture(elements),
        m_texture(Texture::create(m_atlasSize.x, m_atlasSize.y, Texture::Format::Rgba, Sampler::Filter::Nearest))
    {
        Logger::info("Creating font atlas of ", m_atlasSize.x, "x", m_atlasSize.y);
    }
    void add(uint32_t id, uint32_t width, uint32_t height, uint8_t* data)
    {
        ASSERT(width <= m_elementSize.x, "Element too big on x");
        ASSERT(height <= m_elementSize.y, "Element too big on y");
        ASSERT(id < m_elements, "ID out of bound");
        uint32_t idx = id % m_elementCount.x;
        uint32_t idy = id / m_elementCount.x;
        for (uint32_t y = 0; y < height; y++)
        {
            uint32_t yy = idy * m_elementSize.y + y;
            for (uint32_t x = 0; x < width; x++)
            {
                uint32_t xx = idx * m_elementSize.x + x;
                uint32_t index = xx + yy * m_atlasSize.x;
                m_data[index * 4 + 0] = data[y * width + x];
                m_data[index * 4 + 1] = data[y * width + x];
                m_data[index * 4 + 2] = data[y * width + x];
                m_data[index * 4 + 3] = data[y * width + x];
            }
        }
        SubTexture &subTexture = m_subTexture[id];
        subTexture.texture = m_texture;
        subTexture.region.x = static_cast<float>(idx * m_elementSize.x);
        subTexture.region.y = static_cast<float>(idy * m_elementSize.y);
        subTexture.region.w = static_cast<float>(width);
        subTexture.region.h = static_cast<float>(height);
        subTexture.update();
    }
    Texture::Ptr pack()
    {
        m_texture->upload(m_data.data());
        return m_texture;
    }
    SubTexture get(uint32_t id)
    {
        return m_subTexture[id];
    }
private:
    uint32_t m_elements;
    vec2u m_elementCount;
    vec2u m_elementSize;
    vec2u m_atlasSize;
    std::vector<uint8_t> m_data;
private: // output
    std::vector<SubTexture> m_subTexture;
    Texture::Ptr m_texture;
};

Font::Font(const Path& path, uint32_t height)
{
    FT_Library ft;
    FREETYPE_CHECK_RESULT(FT_Init_FreeType(&ft));

    FT_Face face;
    FREETYPE_CHECK_RESULT(FT_New_Face(ft, path.c_str(), 0, &face));
    FREETYPE_CHECK_RESULT(FT_Set_Pixel_Sizes(face, 0, height));

    m_familyName = face->family_name;
    m_styleName = face->style_name;
    m_height = height;

    uint32_t fontMaxAdvance = face->size->metrics.max_advance >> 6;
    uint32_t fontHeight = (face->size->metrics.height >> 6) + 1;
    Packer packer(NUM_GLYPH, fontMaxAdvance, fontHeight);
    for (uint32_t c = 0; c < NUM_GLYPH; c++)
    {
        // FT_Get_Char_Index (if zero returned, missing glyph)
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            Logger::error("[freetype] Failed to load glyph '", (char)c, "'");
            continue;
        }
        packer.add(c, face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer);
        // now store character for later use
        m_characters[c] = {
            vec2i(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            vec2i(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<uint32_t>(face->glyph->advance.x >> 6), // bitshift by 6 to get value in pixels (2^6 = 64)
            SubTexture()
        };
    }
    // Generate the atlas and store it.
    Texture::Ptr atlas = packer.pack();
    for (unsigned char c = 0; c < NUM_GLYPH; c++)
        m_characters[c].texture = packer.get(c);
    std::wstring str;
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
    std::string::const_iterator start = text.begin();
    std::string::const_iterator end = text.end();
    while (start < end)
    {
        uint32_t c = utf8::next(start, end);
        const Character& ch = m_characters[c];
        size.x += ch.advance;
        size.y = max(size.y, ch.size.y);
    }
    return size;
}

const Character& Font::getCharacter(uint32_t c) const
{
    ASSERT(c < NUM_GLYPH, "Glyph out of range");
    return m_characters[c];
}

const std::string& Font::family() const
{
    return m_familyName;
}

const std::string& Font::style() const
{
    return m_styleName;
}

uint32_t Font::height() const
{
    return m_height;
}

}