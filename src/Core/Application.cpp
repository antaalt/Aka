#include <Aka/Core/Application.h>

#include <Aka/Platform/PlatformBackend.h>
#include <Aka/Platform/InputBackend.h>
#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/Audio/AudioBackend.h>
#include <Aka/OS/Logger.h>

namespace aka {

Application::Application(View::Ptr view) :
	m_view(view),
	m_running(true),
	m_width(0), 
	m_height(0) 
{
	EventDispatcher<ViewChangedEvent>::emit(ViewChangedEvent{ m_view });
}
Application::~Application()
{
}
void Application::initialize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;
	m_view->onCreate();
}
void Application::destroy()
{
	m_view->onDestroy();
}
void Application::start()
{
	View::Ptr view = m_view;
	EventDispatcher<ViewChangedEvent>::dispatch();
	if (view != m_view)
	{
		view->onDestroy();
		m_view->onCreate();
	}
}
void Application::update(Time::Unit deltaTime)
{
	EventDispatcher<BackbufferResizeEvent>::dispatch();
	m_view->onUpdate(deltaTime);
}
void Application::frame()
{
	m_view->onFrame();
}
void Application::render()
{
	m_view->onRender();
}
void Application::present()
{
	m_view->onPresent();
}
void Application::end()
{
	EventDispatcher<QuitEvent>::dispatch();
}
void Application::onReceive(const BackbufferResizeEvent& event)
{
	m_view->onResize(event.width, event.height);
}
void Application::onReceive(const QuitEvent& event)
{
	m_running = false;
}
void Application::onReceive(const ViewChangedEvent& event)
{
	m_view = event.view;
}
void Application::run(const Config& config)
{
	if (config.app == nullptr)
		throw std::invalid_argument("No app set.");
	Application* app = config.app;
	PlatformBackend::initialize(config);
	GraphicBackend::initialize(config.width, config.height);
	InputBackend::initialize();
	AudioBackend::initialize(config.audio.frequency, config.audio.channels);
	
	Time::Unit timestep = Time::Unit::milliseconds(10);
	Time::Unit maxUpdate = Time::Unit::milliseconds(100);

	app->initialize(config.width, config.height);

	{
		Time::Unit lastTick = Time::now();
		Time::Unit accumulator = Time::zero();
		do {
			Time::Unit now = Time::now();
			Time::Unit deltaTime = min(now - lastTick, maxUpdate);
			lastTick = now;
			accumulator += deltaTime;
			app->start();
			while (app->m_running && accumulator >= timestep)
			{
				InputBackend::update();
				PlatformBackend::update();
				app->update(timestep);
				accumulator -= timestep;
			}
			GraphicBackend::frame();
			app->frame();
			app->render();
			app->present();
			GraphicBackend::present();
			app->end();
		} while (app->m_running && PlatformBackend::running());
	}

	app->destroy();

	AudioBackend::destroy();
	InputBackend::destroy();
	GraphicBackend::destroy();
	PlatformBackend::destroy();
}

};