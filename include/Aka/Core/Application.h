#pragma once

#include <Aka/Platform/Platform.h>
#include <Aka/Scene/World.h>

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

class Application
{
public:
	Application() : m_running(true), m_width(0), m_height(0) {}
	virtual ~Application() {}
private:
	// Initialize the application and its resources.
	virtual void initialize() {}
	// Destroy everything related to the app.
	virtual void destroy() {}
	// First function called in a loop
	virtual void start() {}
	// Update the app. Might be called multiple for a single frame
	virtual void update(Time::Unit deltaTime) {}
	// Called before render for the app
	virtual void frame() {}
	// Render the app.
	virtual void render() {}
	// Called before present of the frame
	virtual void present() {}
	// Last function called in a loop
	virtual void end() {}
	// Called on app resize
	virtual void resize(uint32_t width, uint32_t height) {}
public:
	// Request to quit the app
	void quit() { m_running = false; }
	// Get the draw width of the app
	uint32_t width() const { return m_width; }
	// Get the draw height of the app
	uint32_t height() const { return m_height; }
public:
	// Entry point of the application
	static void run(const Config& config);
private:
	bool m_running; // Is the app running
	uint32_t m_width;
	uint32_t m_height;
};

}