#include "Backbuffer.h"

namespace aka {

Backbuffer::Backbuffer()
{
}

Backbuffer::Ptr Backbuffer::create()
{
	return std::make_shared<Backbuffer>();
}

void Backbuffer::setSize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;
}

}