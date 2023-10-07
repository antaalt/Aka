#include <Aka/Renderer/InstanceRenderer.hpp>

#include <Aka/Renderer/Renderer.hpp>

namespace aka {

InstanceRenderer::InstanceRenderer(Renderer& _renderer) : 
	m_renderer(_renderer)
{
}

gfx::GraphicDevice* InstanceRenderer::getDevice()
{
	return m_renderer.getDevice();
}

Renderer& InstanceRenderer::getRenderer()
{
	return m_renderer;
}

};