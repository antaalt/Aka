#include <Aka/Core/Application.h>

#include <Aka/Platform/PlatformDevice.h>
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
void Application::initialize(uint32_t width, uint32_t height, int argc, char* argv[])
{
	m_width = width;
	m_height = height;
	onCreate(argc, argv);
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
void Application::update(Time deltaTime)
{
	onUpdate(deltaTime);
	EventDispatcher<BackbufferResizeEvent>::dispatch();
	for (Layer* layer : m_layers)
		layer->onLayerUpdate(deltaTime);
}
void Application::fixedUpdate(Time deltaTime)
{
	onFixedUpdate(deltaTime);
	for (Layer* layer : m_layers)
		layer->onLayerFixedUpdate(deltaTime);
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
	AudioBackend::initialize(config.audio.frequency, config.audio.channels);
	Renderer2D::initialize();
	Renderer3D::initialize();
	
	Time timestep = Time::milliseconds(10);
	Time maxUpdate = Time::milliseconds(100);
	PlatformDevice* platform = PlatformBackend::get();
	GraphicDevice* device = GraphicBackend::device();
	Application* app = config.app;
	app->initialize(config.width, config.height, config.arguments.count, config.arguments.values);

	{
		Time lastTick = Time::now();
		Time accumulator = Time::zero();
		do {
			Time now = Time::now();
			Time deltaTime = min(now - lastTick, maxUpdate);
			lastTick = now;
			accumulator += deltaTime;

			app->start();
			// Update
			while (app->m_running && accumulator >= timestep)
			{
				app->fixedUpdate(timestep);
				accumulator -= timestep;
			}
			platform->poll();
			app->update(deltaTime);
			// Rendering
			device->frame();
			Renderer2D::frame();
			Renderer3D::frame();
			app->frame();
			app->render();
			app->present();
			device->present();

			app->end();
		} while (app->m_running);
	}

	app->destroy();

	Renderer3D::destroy();
	Renderer2D::destroy();
	AudioBackend::destroy();
	GraphicBackend::destroy();
	PlatformBackend::destroy();
}

};