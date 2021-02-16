#include <Aka/Core/Application.h>

#include <Aka/Platform/PlatformBackend.h>
#include <Aka/Platform/InputBackend.h>
#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/Audio/AudioBackend.h>

namespace aka {

void Application::run(const Config& config)
{
	Application* app = config.app;
	PlatformBackend::initialize(config);
	GraphicBackend::initialize(config.width, config.height);
	InputBackend::initialize();
	AudioBackend::initialize(44100, 2);
	
	Time::Unit timestep = Time::Unit::milliseconds(10);
	Time::Unit maxUpdate = Time::Unit::milliseconds(100);

	app->m_width = config.width;
	app->m_height = config.height;
	app->initialize();

	{
		Time::Unit lastTick = Time::now();
		Time::Unit accumulator = Time::Unit::milliseconds(0);
		do {
			Time::Unit now = Time::now();
			Time::Unit deltaTime = min<Time::Unit>(now - lastTick, maxUpdate);
			lastTick = now;
			accumulator += deltaTime;
			while (app->m_running && accumulator >= timestep)
			{
				InputBackend::update();
				PlatformBackend::update();
				uint32_t w, h;
				GraphicBackend::getSize(&w, &h);
				if (w != app->m_width || h != app->m_height)
				{
					app->m_width = w;
					app->m_height = h;
					app->resize(w, h);
				}
				app->update(timestep);
				accumulator -= timestep;
			}
			GraphicBackend::frame();
			app->frame();
			app->render();
			GraphicBackend::present();
		} while (app->m_running && PlatformBackend::running());
	}

	app->destroy();

	AudioBackend::destroy();
	InputBackend::destroy();
	GraphicBackend::destroy();
	PlatformBackend::destroy();
}

};