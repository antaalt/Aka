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
	std::string name = "Aka";
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

class Application final : 
	EventListener<QuitEvent>, 
	EventListener<ViewChangedEvent>, 
	EventListener<BackbufferResizeEvent>
{
public:
	Application(View::Ptr view);
	~Application();
private:
	// Initialize the application and its resources.
	void initialize(uint32_t width, uint32_t height);
	// Destroy everything related to the app.
	void destroy();
	// First function called in a loop
	void start();
	// Update the app. Might be called multiple for a single frame
	void update(Time::Unit deltaTime);
	// Called before render for the app
	void frame();
	// Render the app.
	void render();
	// Called before present of the frame
	void present();
	// Last function called in a loop
	void end();
	// Called on app resize
	void onReceive(const BackbufferResizeEvent& event);
	// Called on app quit request
	void onReceive(const QuitEvent& event);
	// Called on app view change
	void onReceive(const ViewChangedEvent& event);
public:
	// Entry point of the application
	static void run(const Config& config);
private:
	View::Ptr m_view;
	bool m_running; // Is the app running
	uint32_t m_width;
	uint32_t m_height;
};

}