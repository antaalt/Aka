#include "Application.h"

#include "../Platform/PlatformBackend.h"
#include "../Platform/InputBackend.h"
#include "../Graphic/GraphicBackend.h"
#include "../Audio/AudioBackend.h"

namespace aka {

void Application::run(const Config& config)
{
	Application* app = config.app;
	PlatformBackend::initialize(config);
	GraphicBackend::initialize(config.width, config.height);
	InputBackend::initialize();
	AudioBackend::initialize(44100, 2);

	app->initialize();

	{
		Time::Unit lastTick = Time::now();
		Time::Unit accumulator;
		Time::Unit timestep = Time::Unit::milliseconds(10);
		do {
			Time::Unit now = Time::now();
			Time::Unit deltaTime = min<Time::Unit>(now - lastTick, Time::Unit::milliseconds(100));
			lastTick = now;
			accumulator += deltaTime;
			while (accumulator >= timestep)
			{
				InputBackend::frame();
				PlatformBackend::frame();
				app->update(timestep);
				accumulator -= timestep;
			}
			GraphicBackend::frame();
			app->frame();
			app->render();
			GraphicBackend::present();
			PlatformBackend::present();
		} while (app->running() && PlatformBackend::running());
	}

	app->destroy();

	AudioBackend::destroy();
	InputBackend::destroy();
	GraphicBackend::destroy();
	PlatformBackend::destroy();
}

};