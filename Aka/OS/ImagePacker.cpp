#include "ImagePacker.h"

namespace aka {

Packer::Packer(uint32_t elements, uint32_t elementWidth, uint32_t elementHeight) :
    m_packed(false),
    m_elements(elements),
    m_elementCount((uint32_t)ceil<float>(sqrt<float>((float)elements)), (uint32_t)floor<float>(sqrt<float>((float)elements) + 0.5f)),
    m_elementSize(elementWidth, elementHeight),
    m_image((m_elementCount* m_elementSize).x, (m_elementCount* m_elementSize).y, 4),
    m_regions(elements)
{
    ASSERT(m_image.width < 4096, "Atlas too wide.");
    ASSERT(m_image.height < 4096, "Atlas too high.");
}
void Packer::add(uint32_t id, const Image& image)
{
    if (m_packed) return;
    ASSERT(image.components == 4, "Does not support image with less than 4 components");
    add(id, image.width, image.height, image.bytes.data());
}
void Packer::add(uint32_t id, uint32_t width, uint32_t height, const uint8_t* data)
{
    if (m_packed) return;
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
            uint32_t index = xx + yy * m_image.width;
            m_image.bytes[index * 4 + 0] = data[y * width + x];
            m_image.bytes[index * 4 + 1] = data[y * width + x];
            m_image.bytes[index * 4 + 2] = data[y * width + x];
            m_image.bytes[index * 4 + 3] = data[y * width + x];
        }
    }
    Rect& region = m_regions[id];
    region.x = static_cast<int32_t>(idx * m_elementSize.x);
    region.y = static_cast<int32_t>(m_image.height - height - idy * m_elementSize.y);
    region.w = width;
    region.h = height;
}

const Image& Packer::pack()
{
    if (m_packed) return m_image;
#if defined(ORIGIN_BOTTOM_LEFT)
    m_image.flip();
#endif
    m_packed = true;
    return m_image;
}

const Image& Packer::getImage() const
{
    return m_image;
}

Rect Packer::getRegion(uint32_t id) const
{
    return m_regions[id];
}

};