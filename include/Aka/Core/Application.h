#pragma once

#include <Aka/Platform/Platform.h>
#include <Aka/Platform/PlatformBackend.h>
#include <Aka/Scene/World.h>

#include <Aka/Core/Event.h>
#include <Aka/Core/View.h>

namespace aka {

class Application;

struct Config {
	uint32_t width = 1280;
	uint32_t height = 720;
	String name = "Aka";
	Application* app = nullptr;
	struct Audio {
		uint32_t frequency = 44100;
		uint32_t channels = 2;
	} audio;
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
	void initialize(uint32_t width, uint32_t height);
	// Destroy everything related to the app.
	void destroy();
	// First function called in a loop
	void start();
	// Update the app. Might be called multiple for a single frame
	void update(Time::Unit deltaTime);
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
	virtual void onCreate() {}
	// Called on app destruction.
	virtual void onDestroy() {}
	// Called on app update
	virtual void onUpdate(Time::Unit deltaTime) {}
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
public:
	// Entry point of the application
	static void run(const Config& config);
private:
	uint32_t m_width, m_height;
	bool m_running;
};

}