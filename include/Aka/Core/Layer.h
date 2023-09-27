#pragma once

#include <Aka/OS/Time.h>
#include <Aka/Core/Application.h>

namespace aka {

class Layer
{
public:
	Layer();
	virtual ~Layer();
	template <typename T, typename... Args> T* addLayer(Args&&... args);
private:
	friend class Application;
	void create(gfx::GraphicDevice* _device);
	void destroy(gfx::GraphicDevice* _device);
	void fixedUpdate(Time delta);
	void update(Time delta);
	void preRender();
	void render(gfx::GraphicDevice* _device, gfx::Frame* frame);
	void postRender();
	void resize(uint32_t width, uint32_t height);
protected:
	virtual void onLayerCreate(gfx::GraphicDevice* _device) {}
	virtual void onLayerDestroy(gfx::GraphicDevice* _device) {}

	virtual void onLayerUpdate(Time deltaTime) {}
	virtual void onLayerFixedUpdate(Time deltaTime) {}
	virtual void onLayerPreRender() {}
	virtual void onLayerRender(aka::gfx::GraphicDevice* _device, gfx::Frame* frame) {}
	virtual void onLayerPostRender() {}

	virtual void onLayerResize(uint32_t width, uint32_t height) {}
private:
	Vector<Layer*> m_childrens;
};


template <typename T, typename... Args>
inline T* Layer::addLayer(Args&&... args)
{
	static_assert(std::is_base_of<Layer, T>::value, "Type is not a layer");
	T* layer = new T(std::forward<Args>(args)...);
	m_childrens.append(layer);
	// TODO should be called or prevent to call addLayer after constructor...
	//layer->onLayerCreate(*this);
	return layer;
}

};