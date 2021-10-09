#include <Aka/Core/Application.h>

#include <Aka/Core/Layer.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/ProgramManager.h>
#include <Aka/Resource/ResourceManager.h>
#include <Aka/Drawing/Renderer2D.h>
#include <Aka/Drawing/Renderer3D.h>
#include <Aka/Audio/AudioDevice.h>
#include <Aka/OS/Logger.h>

#include "Platform/GLFW3/PlatformGLFW3.h"
#include "Graphic/D3D11/D3D11Device.h"
#include "Graphic/GL/GLDevice.h"
#include "Audio/RtAudio/AudioRtAudio.h"

namespace aka {

PlatformDevice* platformFactory(const PlatformConfig& config)
{
#if defined(AKA_USE_GLFW3)
	return new PlatformGLFW3(config);
#elif
	return nullptr;
#endif
}

GraphicDevice* graphicFactory(PlatformDevice* platform, const GraphicConfig& config)
{
#if defined(AKA_USE_OPENGL)
	return new GLDevice(platform, config);
#elif defined(AKA_USE_D3D11)
	return new D3D11Device(platform, config);
#else
	return nullptr;
#endif
}

AudioDevice* audioFactory(const AudioConfig& config)
{
#if defined(AKA_USE_RTAUDIO)
	return new AudioRtAudio(config);
#else
	return nullptr;
#endif
}

Application::Application(const std::vector<Layer*> layers) :
	m_width(0),
	m_height(0),
	m_running(true),
	m_layers(layers)
{
}
Application::~Application()
{
	for (Layer* layer : m_layers)
		delete layer;
}
void Application::create(const Config& config)
{
	s_platform = platformFactory(config.platform);
	s_graphic = graphicFactory(s_platform, config.graphic);
	s_audio = audioFactory(config.audio);
	s_program = new ProgramManager;
	s_resource = new ResourceManager;
	m_width = config.platform.width;
	m_height = config.platform.height;
	onCreate(config.argc, config.argv);
	EventDispatcher<AppCreateEvent>::trigger(AppCreateEvent{});
}
void Application::destroy()
{
	EventDispatcher<AppDestroyEvent>::trigger(AppDestroyEvent{});
	onDestroy();

	delete s_audio;
	delete s_graphic;
	delete s_platform;
	delete s_program;
	delete s_resource;
	
	s_audio = nullptr;
	s_graphic = nullptr;
	s_platform = nullptr;
	s_program = nullptr;
	s_resource = nullptr;
}
void Application::start()
{
}
void Application::update(Time deltaTime)
{
	onUpdate(deltaTime);
	EventDispatcher<AppUpdateEvent>::trigger(AppUpdateEvent{ deltaTime });
}
void Application::fixedUpdate(Time deltaTime)
{
	onFixedUpdate(deltaTime);
	EventDispatcher<AppFixedUpdateEvent>::trigger(AppFixedUpdateEvent{ deltaTime });
}
void Application::frame()
{
	onFrame();
	EventDispatcher<AppFrameEvent>::trigger(AppFrameEvent{});
}
void Application::render()
{
	onRender();
	EventDispatcher<AppRenderEvent>::trigger(AppRenderEvent{});
}
void Application::present()
{
	onPresent();
	EventDispatcher<AppPresentEvent>::trigger(AppPresentEvent{});
}
void Application::end()
{
}
void Application::onReceive(const WindowResizeEvent& event)
{
	onResize(event.width, event.height);
	EventDispatcher<AppResizeEvent>::trigger(AppResizeEvent{ event.width, event.height});
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

PlatformDevice* Application::s_platform = nullptr;
GraphicDevice* Application::s_graphic = nullptr;
AudioDevice* Application::s_audio = nullptr;
ProgramManager* Application::s_program = nullptr;
ResourceManager* Application::s_resource = nullptr;

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

ProgramManager* Application::program()
{
	return s_program;
}

ResourceManager* Application::resource()
{
	return s_resource;
}

void Application::run(const Config& config)
{
	if (config.app == nullptr)
		throw std::invalid_argument("No app set.");
	
	const Time timestep = Time::milliseconds(10);
	const Time maxUpdate = Time::milliseconds(100);

	Application* app = config.app;

	app->create(config);

	GraphicDevice* graphic = s_graphic;
	PlatformDevice* platform = s_platform;

	Renderer2D::initialize();
	Renderer3D::initialize();
	
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
		graphic->frame();
		Renderer2D::frame();
		Renderer3D::frame();
		app->frame();
		app->render();
		app->present();
		graphic->present();

		app->end();
		EventDispatcher<QuitEvent>::dispatch();
	} while (app->m_running);

	Renderer3D::destroy();
	Renderer2D::destroy();

	app->destroy();
}

};