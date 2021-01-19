#include "Platform/Window.h"

#include "Game.h"

// TODO use cmake
// implement other backend
// add 3D

int main()
{
	aka::Game app;

	aka::Window::Config cfg;
	cfg.width = 1280;
	cfg.height = 720;
	cfg.name = "Game";
	cfg.api = aka::GraphicBackend::Api::OpenGL;
	cfg.app = &app;

	aka::Window::run(cfg);

	return 0;
}