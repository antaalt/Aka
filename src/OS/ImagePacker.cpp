#include <Aka/OS/ImagePacker.h>

namespace aka {

Packer::Packer(uint32_t elements, uint32_t elementWidth, uint32_t elementHeight) :
    m_packed(false),
    m_elements(elements),
    m_elementCount((uint32_t)ceil<float>(sqrt<float>((float)elements)), (uint32_t)floor<float>(sqrt<float>((float)elements) + 0.5f)),
    m_elementSize(elementWidth, elementHeight),
    m_image((m_elementCount* m_elementSize).x, (m_elementCount* m_elementSize).y, 4, ImageFormat::UnsignedByte),
    m_regions(elements)
{
	AKA_ASSERT(m_image.width() < 4096, "Atlas too wide.");
	AKA_ASSERT(m_image.height() < 4096, "Atlas too high.");
}
void Packer::add(uint32_t id, const Image& image)
{
    if (m_packed) return;
    add(id, image.width(), image.height(), static_cast<const uint8_t*>(image.data()), image.components());
}
void Packer::add(uint32_t id, uint32_t width, uint32_t height, const uint8_t* data, uint8_t components)
{
	// TODO check component count is 1.
    if (m_packed) return;
	AKA_ASSERT(width <= m_elementSize.x, "Element too big on x");
	AKA_ASSERT(height <= m_elementSize.y, "Element too big on y");
	AKA_ASSERT(id < m_elements, "ID out of bound");
    uint32_t idx = id % m_elementCount.x;
    uint32_t idy = id / m_elementCount.x;
	uint8_t index[4] = { 1, 2, 3, 4	};
	if (components != 4)
	{
		if (components == 1)
			for (uint32_t x = 0; x < 4; x++)
				index[x] = 0;
		else if (components == 3)
			index[3] = 0;
		else
			return; // not supported
	}
	for (uint32_t y = 0; y < height; y++)
	{
		uint32_t yy = idy * m_elementSize.y + y;
		for (uint32_t x = 0; x < width; x++)
		{
			uint32_t xx = idx * m_elementSize.x + x;
			m_image.set(xx, yy, color32(
				data[y * width * components + x * components + index[0]],
				data[y * width * components + x * components + index[1]],
				data[y * width * components + x * components + index[2]],
				data[y * width * components + x * components + index[3]]
			));
		}
	}
    Rect& region = m_regions[id];
    region.x = static_cast<int32_t>(idx * m_elementSize.x);
    region.y = static_cast<int32_t>(m_image.height() - height - idy * m_elementSize.y);
    region.w = width;
    region.h = height;
}

const Image& Packer::pack()
{
    if (m_packed) return m_image;
#if defined(AKA_ORIGIN_BOTTOM_LEFT)
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