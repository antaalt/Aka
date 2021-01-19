#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <memory>

#include "../Platform/Platform.h"
#include "../Graphic/GraphicBackend.h"

// https://www.gamedesigning.org/learn/make-a-game-engine/
namespace aka {

class Application;

class Window {
public:
	struct Config {
		uint32_t width;
		uint32_t height;
		std::string name;
		GraphicBackend::Api api;
		Application* app;
	};
private:
	Window(const Config& config);
	~Window();
	// Loop
	void loop();
public:
	static void run(const Window::Config& config);

	// Set window size limits
	void setSizeLimits(int32_t minWidth, int32_t minHeight, int32_t maxWidth, int32_t maxHeight);

	// Get window handle
	GLFWwindow* handle() const;

private:
	// TODO : Abstraction depending on window system (SDL, GLFW...)
	Application* m_app;
	GLFWwindow* m_window;
	GraphicBackend m_backend;
};

}
