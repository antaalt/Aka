#pragma once

#include "../Graphic/GraphicBackend.h"
#include "ECS/World.h"

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
	// Initialize the app, set the listener on inputs, create resources...
	virtual void initialize() {};
	// Destroy everything related to the app.
	virtual void destroy() {};
	// Create a new frame for the app
	virtual void frame() {};
	// Update the app.
	virtual void update(Time::Unit deltaTime) {};
	// Render the app.
	virtual void render() {};
	// Is the app running
	virtual bool running() { return true; };
public:
	// Entry point of the application
	static void run(const Config& config);
};

}