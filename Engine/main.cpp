#include "Window.h"

#include "Game.h"

// TODO use cmake
// implement other backend
// add 3D

int main()
{
	app::Game app;

	app::Window::Config cfg;
	cfg.width = 1280;
	cfg.height = 720;
	cfg.name = "Game";
	cfg.api = app::GraphicBackend::API::OPENGL;
	cfg.app = &app;

	app::Window::run(cfg);

	return 0;
}