#pragma once

#include <Aka/Platform/Platform.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Scene/World.h>

#include <Aka/Core/Event.h>
#include <Aka/Core/View.h>
#include <Aka/Core/Layer.h>

namespace aka {

class Application;

struct Config {
	uint32_t width = 1280;
	uint32_t height = 720;
	String name = "Aka";
	Image icon = {};
	Application* app = nullptr;
	struct Audio {
		uint32_t frequency = 44100;
		uint32_t channels = 2;
	} audio;
	struct Arguments {
		int count = 0;
		char** values = nullptr;
	} arguments;
};

// Event to notify app to exit.
struct QuitEvent {};

// Event to notify a view change
struct ViewChangedEvent
{
	View::Ptr view;
};

class Application : 
	EventListener<QuitEvent>,
	EventListener<BackbufferResizeEvent>
{
public:
	Application();
	virtual ~Application();
private:
	// Initialize the application and its resources.
	void initialize(uint32_t width, uint32_t height, int argc, char* argv[]);
	// Destroy everything related to the app.
	void destroy();
	// First function called in a loop
	void start();
	// Update the app. deltaTime is time elapsed since last frame
	void update(Time deltaTime);
	// Update the app multiple time per frame at a fixed timestep
	void fixedUpdate(Time deltaTime);
	// Called before render for the app.
	void frame();
	// Render the app.
	void render();
	// Called before present of the frame
	void present();
	// Last function called in a loop
	void end();
	// Called on app resize
	void onReceive(const BackbufferResizeEvent& event) override;
	// Called on app quit request
	void onReceive(const QuitEvent& event) override;
protected:
	// Called on app creation.
	virtual void onCreate(int argc, char* argv[]) {}
	// Called on app destruction.
	virtual void onDestroy() {}
	// Called on app update
	virtual void onUpdate(Time deltaTime) {}
	// Called on app update at a fixed timestep
	virtual void onFixedUpdate(Time deltaTime) {}
	// Called before app render
	virtual void onFrame() {}
	// Called on app render
	virtual void onRender() {}
	// Called before present of the app
	virtual void onPresent() {}
	// Called on app resize
	virtual void onResize(uint32_t width, uint32_t height) {}
	// Get the current app width
	uint32_t width() const;
	// Get the current app height
	uint32_t height() const;
	// Attach a layer to the app
	template <typename T> void attach();
	// Detach a layer from the app
	template <typename T> void detach();
	// Get a layer from the app
	template <typename T> T& get();
public:
	// Entry point of the application
	static void run(const Config& config);
private:
	std::vector<Layer*> m_layers;
	uint32_t m_width, m_height;
	bool m_running;
};

template <typename T>
void Application::attach() 
{
	static_assert(std::is_base_of<Layer, T>::value, "Type is not a layer.");
	for (Layer* layer : m_layers)
		if (typeid(*layer) == typeid(T))
			return; // already attached.
	m_layers.push_back(new T);
	m_layers.back()->onLayerAttach();
}
template <typename T>
void Application::detach()
{
	static_assert(std::is_base_of<Layer, T>::value, "Type is not a layer.");
	for (auto it = m_layers.begin(); it != m_layers.end(); it++)
	{
		Layer* layer = *it;
		if (typeid(*layer) == typeid(T))
		{
			layer->onLayerDetach();
			m_layers.erase(it);
			delete layer;
			break;
		}
	}
}
template <typename T>
T& Application::get()
{
	static_assert(std::is_base_of<Layer, T>::value, "Type is not a layer.");
	for (Layer* layer : m_layers)
		if (typeid(*layer) == typeid(T))
			return reinterpret_cast<T&>(*layer);
	throw std::runtime_error("Layer not attached");
}

}