#pragma once

#include <Aka/Platform/Platform.h>
#include <Aka/Core/ECS/World.h>

namespace aka {

class Application;

struct Config {
	uint32_t width;
	uint32_t height;
	std::string name;
	Application* app;
};

class Application
{
public:
	Application() : m_running(true), m_width(0), m_height(0) {}
	virtual ~Application() {}
	// Initialize the app, set the listener on inputs, create resources...
	virtual void initialize() {}
	// Destroy everything related to the app.
	virtual void destroy() {}
	// Create a new frame for the app
	virtual void frame() {}
	// Update the app.
	virtual void update(Time::Unit deltaTime) {}
	// Render the app.
	virtual void render() {}
	// On app resize
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