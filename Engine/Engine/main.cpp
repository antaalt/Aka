#include "Window.h"

#include "CustomApp.h"

// TODO use cmake
// implement other backend
// add 3D

int main()
{
	app::CustomApp app;

	app::Window::Config cfg;
	cfg.width = 1280;
	cfg.height = 720;
	cfg.name = "App";
	cfg.api = app::GraphicBackend::API::OPENGL;
	cfg.app = &app;

	app::Window::run(cfg);

	return 0;
}