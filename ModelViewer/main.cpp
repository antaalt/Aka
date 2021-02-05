#include "ModelViewer.h"

int main(int argc, char* argv[])
{
	viewer::Viewer viewer;

	aka::Config cfg{};
	cfg.app = &viewer;
	cfg.width = 1280;
	cfg.height = 720;
	cfg.name = "Model Viewer";
	aka::Application::run(cfg);

	return 0;
}