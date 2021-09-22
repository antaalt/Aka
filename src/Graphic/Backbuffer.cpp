#pragma once

#include <Aka/Graphic/Backbuffer.h>

namespace aka {

Backbuffer::Backbuffer(uint32_t width, uint32_t height) :
	Framebuffer(width, height)
{
}

Backbuffer::~Backbuffer()
{
}

void Backbuffer::onReceive(const BackbufferResizeEvent& event)
{
	m_width = event.width;
	m_height = event.height;
	resize(m_width, m_height);
}

};