#include <Aka/Core/Layer.h>

namespace aka {

Layer::Layer()
{
}
Layer::~Layer()
{
	for (Layer* layer : m_childrens)
		delete layer;
}
void Layer::create()
{
	onLayerCreate();
	for (Layer* children : m_childrens)
		children->create();
}
void Layer::destroy() 
{
	for (Layer* children : m_childrens)
		children->destroy();
	onLayerDestroy();
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
void Layer::render(gfx::Frame* frame)
{
	for (Layer* children : m_childrens)
		children->render(frame);
	onLayerRender(frame);
}
void Layer::frame() 
{
	onLayerFrame();
	for (Layer* children : m_childrens)
		children->frame();
}
void Layer::present()
{
	for (Layer* children : m_childrens)
		children->present();
	onLayerPresent();
}
void Layer::resize(uint32_t width, uint32_t height)
{
	onLayerResize(width, height);
	for (Layer* children : m_childrens)
		children->resize(width, height);
}
}; // namespace aka