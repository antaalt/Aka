#include <Aka/Core/Application.h>

#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Audio/AudioDevice.h>
#include <Aka/OS/Logger.h>

#include "Platform/GLFW3/PlatformGLFW3.h"
#include "Graphic/D3D11/D3D11Device.h"
#include "Graphic/GL/GLDevice.h"
#include "Audio/RtAudio/AudioRtAudio.h"

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

GraphicDevice* Application::s_graphic = nullptr;
PlatformDevice* Application::s_platform = nullptr;
AudioDevice* Application::s_audio = nullptr;


GraphicDevice* Application::graphic()
{
	return s_graphic;
}

PlatformDevice* Application::platform()
{
	return s_platform;
}

AudioDevice* Application::audio()
{
	return s_audio;
}

void Application::run(const Config& config)
{
	if (config.app == nullptr)
		throw std::invalid_argument("No app set.");

#if defined(AKA_USE_GLFW3)
	s_platform = new PlatformGLFW3(config.platform);
#endif
#if defined(AKA_USE_OPENGL)
	s_graphic = new GLDevice(config.graphic);
#elif defined(AKA_USE_D3D11)
	s_graphic = new D3D11Device(config.graphic);
#endif
	s_audio = new AudioRtAudio(config.audio);

	Application* app = config.app;

	Renderer2D::initialize();
	Renderer3D::initialize();
	
	Time timestep = Time::milliseconds(10);
	Time maxUpdate = Time::milliseconds(100);
	app->initialize(config.platform.width, config.platform.height, config.argc, config.argv);

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
			s_platform->poll();
			app->update(deltaTime);
			// Rendering
			s_graphic->frame();
			Renderer2D::frame();
			Renderer3D::frame();
			app->frame();
			app->render();
			app->present();
			s_graphic->present();

			app->end();
		} while (app->m_running);
	}

	app->destroy();

	Renderer3D::destroy();
	Renderer2D::destroy();
	delete s_audio;
	delete s_graphic;
	delete s_platform;
	s_audio = nullptr;
	s_graphic = nullptr;
	s_platform = nullptr;
}

};