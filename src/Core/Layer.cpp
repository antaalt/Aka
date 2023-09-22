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
void Layer::create(gfx::GraphicDevice* _device)
{
	onLayerCreate(_device);
	for (Layer* children : m_childrens)
		children->create(_device);
}
void Layer::destroy(gfx::GraphicDevice* _device)
{
	for (Layer* children : m_childrens)
		children->destroy(_device);
	onLayerDestroy(_device);
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
void Layer::render(gfx::GraphicDevice* _device, gfx::Frame* frame)
{
	for (Layer* children : m_childrens)
		children->render(_device, frame);
	onLayerRender(_device, frame);
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