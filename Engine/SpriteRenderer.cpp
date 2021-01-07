#include "SpriteRenderer.h"


namespace app {


void SpriteRenderer::viewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
}

};