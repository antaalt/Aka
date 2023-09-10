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
	void create();
	void destroy();
	void fixedUpdate(Time delta);
	void update(Time delta);
	void render(gfx::Frame* frame);
	void frame();
	void present();
	void resize(uint32_t width, uint32_t height);
protected:
	virtual void onLayerCreate() {}
	virtual void onLayerDestroy() {}

	virtual void onLayerUpdate(Time deltaTime) {}
	virtual void onLayerFixedUpdate(Time deltaTime) {}
	virtual void onLayerFrame() {}
	virtual void onLayerRender(gfx::Frame* frame) {}
	virtual void onLayerPresent() {}

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