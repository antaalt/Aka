#pragma once

#include <Aka/OS/Time.h>
#include <Aka/Core/Application.h>

namespace aka {

class PlatformWindow;

class Layer
{
public:
	explicit Layer(PlatformWindow* window);
	Layer(const Layer&) = delete;
	Layer(Layer&&) = delete;
	Layer& operator=(const Layer&) = delete;
	Layer& operator=(Layer&&) = delete;
	virtual ~Layer();

	// Add a child layer to this layer.
	template <typename T, typename... Args> T* addLayer(Args&&... args);
	// Set the window of this layer.
	void setWindow(PlatformWindow* window) { m_window = window; }
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

	PlatformWindow* getWindow() const { return m_window; }
private:
	PlatformWindow* m_window;
	Vector<Layer*> m_childrens;
};


template <typename T, typename... Args>
inline T* Layer::addLayer(Args&&... args)
{
	static_assert(std::is_base_of<Layer, T>::value, "Type is not a layer");
	T* layer = mem::akaNew<T>(AllocatorMemoryType::Object, AllocatorCategory::Global, std::forward<Args>(args)...);
	m_childrens.append(layer);
	// TODO should be called or prevent to call addLayer after constructor...
	//layer->onLayerCreate(*this);
	return layer;
}

};