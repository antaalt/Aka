#include <Aka/Core/Application.h>

#include <Aka/Platform/PlatformBackend.h>
#include <Aka/Platform/InputBackend.h>
#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/Audio/AudioBackend.h>
#include <Aka/OS/Logger.h>

namespace aka {

Application::Application() :
	m_width(0),
	m_height(0),
	m_running(true)
{
}
Application::~Application()
{
}
void Application::initialize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;
	onCreate();
}
void Application::destroy()
{
	for (Layer* layer : m_layers)
		layer->onLayerDetach();
	m_layers.clear();
	onDestroy();
}
void Application::start()
{
}
void Application::update(Time::Unit deltaTime)
{
	onUpdate(deltaTime);
	EventDispatcher<BackbufferResizeEvent>::dispatch();
	for (Layer* layer : m_layers)
		layer->onLayerUpdate(deltaTime);
}
void Application::frame()
{
	for (Layer* layer : m_layers)
		layer->onLayerFrame();
	onFrame();
}
void Application::render()
{
	for (Layer* layer : m_layers)
		layer->onLayerRender();
	onRender();
}
void Application::present()
{
	for (Layer* layer : m_layers)
		layer->onLayerPresent();
	onPresent();
}
void Application::end()
{
	EventDispatcher<QuitEvent>::dispatch();
}
void Application::onReceive(const BackbufferResizeEvent& event)
{
	onResize(event.width, event.height);
	for (Layer* layer : m_layers)
		layer->onLayerResize(event.width, event.height);
	m_width = event.width;
	m_height = event.height;
}
void Application::onReceive(const QuitEvent& event)
{
	m_running = false;
}

uint32_t Application::width() const
{
	return m_width;
}

uint32_t Application::height() const
{
	return m_height;
}

void Application::run(const Config& config)
{
	if (config.app == nullptr)
		throw std::invalid_argument("No app set.");
	PlatformBackend::initialize(config);
	GraphicBackend::initialize(config.width, config.height);
	InputBackend::initialize();
	AudioBackend::initialize(config.audio.frequency, config.audio.channels);
	Renderer2D::initialize();
	Renderer3D::initialize();
	
	Time::Unit timestep = Time::Unit::milliseconds(10);
	Time::Unit maxUpdate = Time::Unit::milliseconds(100);

	Application* app = config.app;
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
			Renderer2D::frame();
			Renderer3D::frame();
			app->frame();
			app->render();
			app->present();
			GraphicBackend::present();
			app->end();
		} while (app->m_running && PlatformBackend::running());
	}

	app->destroy();

	Renderer3D::destroy();
	Renderer2D::destroy();
	AudioBackend::destroy();
	InputBackend::destroy();
	GraphicBackend::destroy();
	PlatformBackend::destroy();
}

};