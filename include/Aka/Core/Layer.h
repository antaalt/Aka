#pragma once

#include <Aka/OS/Time.h>
#include <Aka/Core/Application.h>

namespace aka {

class Layer
{
public:
	Layer();
	Layer(const Layer&) = delete;
	Layer(Layer&&) = delete;
	Layer& operator=(const Layer&) = delete;
	Layer& operator=(Layer&&) = delete;
	virtual ~Layer();
	template <typename T, typename... Args> T* addLayer(Args&&... args);
private:
	friend class Application;
	void create(Renderer* _renderer);
	void destroy(Renderer* _renderer);
	void fixedUpdate(Time delta);
	void update(Time delta);
	void preRender();
	void render(Renderer* _renderer, gfx::FrameHandle frame);
	void postRender();
	void resize(uint32_t width, uint32_t height);
protected:
	virtual void onLayerCreate(Renderer* _renderer) {}
	virtual void onLayerDestroy(Renderer* _renderer) {}

	virtual void onLayerUpdate(Time deltaTime) {}
	virtual void onLayerFixedUpdate(Time deltaTime) {}
	virtual void onLayerPreRender() {}
	virtual void onLayerRender(Renderer* _renderer, gfx::FrameHandle frame) {}
	virtual void onLayerPostRender() {}

	virtual void onLayerResize(uint32_t width, uint32_t height) {}
private:
	Vector<Layer*> m_childrens;
};


template <typename T, typename... Args>
inline T* Layer::addLayer(Args&&... args)
{
	static_assert(std::is_base_of<Layer, T>::value, "Type is not a layer");
	T* layer = mem::akaNew<T>(AllocatorMemoryType::Persistent, AllocatorCategory::Default, std::forward<Args>(args)...);
	m_childrens.append(layer);
	// TODO should be called or prevent to call addLayer after constructor...
	//layer->onLayerCreate(*this);
	return layer;
}

};