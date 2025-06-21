#include <Aka/Core/Layer.h>

namespace aka {

Layer::Layer(PlatformWindow* window) :
	m_window(window)
{
}
Layer::~Layer()
{
	for (Layer* layer : m_childrens)
		mem::akaDelete(layer);
}
void Layer::create(Renderer* _renderer)
{
	onLayerCreate(_renderer);
	for (Layer* children : m_childrens)
		children->create(_renderer);
}
void Layer::destroy(Renderer* _renderer)
{
	for (Layer* children : m_childrens)
		children->destroy(_renderer);
	onLayerDestroy(_renderer);
}
void Layer::fixedUpdate(Time deltaTime)
{
	onLayerFixedUpdate(deltaTime);
	for (Layer* children : m_childrens)
		children->fixedUpdate(deltaTime);
}
void Layer::update(Time deltaTime)
{
	onLayerUpdate(deltaTime);
	for (Layer* children : m_childrens)
		children->update(deltaTime);
}
void Layer::render(Renderer* _renderer, gfx::FrameHandle frame)
{
	for (Layer* children : m_childrens)
		children->render(_renderer, frame);
	onLayerRender(_renderer, frame);
}
void Layer::preRender()
{
	onLayerPreRender();
	for (Layer* children : m_childrens)
		children->preRender();
}
void Layer::postRender()
{
	for (Layer* children : m_childrens)
		children->postRender();
	onLayerPostRender();
}
void Layer::resize(uint32_t width, uint32_t height)
{
	onLayerResize(width, height);
	for (Layer* children : m_childrens)
		children->resize(width, height);
}
}; // namespace aka