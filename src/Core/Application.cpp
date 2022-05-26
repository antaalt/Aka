#include <Aka/Core/Application.h>

#include <Aka/Core/Layer.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/ProgramManager.h>
#include <Aka/Audio/AudioDevice.h>
#include <Aka/OS/OS.h>
#include <Aka/OS/Logger.h>

namespace aka {

Application* Application::s_app = nullptr;

Application::Application() :
	Application(std::vector<Layer*>{})
{
}
Application::Application(const std::vector<Layer*> layers) :
	m_platform(nullptr),
	m_graphic(nullptr),
	m_audio(nullptr),
	m_program(nullptr),
	m_registry(nullptr),
	m_layers(layers),
	m_width(0),
	m_height(0),
	m_running(true)
{
}
Application::~Application()
{
	for (Layer* layer : m_layers)
		delete layer;
}
void Application::create(const Config& config)
{
	OS::setcwd(config.directory);
	m_platform = PlatformDevice::create(config.platform);
	AKA_ASSERT(m_platform != nullptr, "No platform");
	m_graphic = gfx::GraphicDevice::create(m_platform, config.graphic);
	AKA_ASSERT(m_graphic != nullptr, "No graphics");
	m_audio = AudioDevice::create(config.audio);
	AKA_ASSERT(m_audio != nullptr, "No audio");
	m_program = new ProgramManager;
	m_registry = new AssetRegistry;
	m_width = config.platform.width;
	m_height = config.platform.height;
	EventDispatcher<AppCreateEvent>::trigger(AppCreateEvent{});
	onCreate(config.argc, config.argv);
}
void Application::destroy()
{
	EventDispatcher<AppDestroyEvent>::trigger(AppDestroyEvent{});
	onDestroy();

	delete m_program;
	delete m_registry;
	AudioDevice::destroy(m_audio);
	gfx::GraphicDevice::destroy(m_graphic);
	PlatformDevice::destroy(m_platform);
	
	m_audio = nullptr;
	m_graphic = nullptr;
	m_platform = nullptr;
	m_program = nullptr;
	m_registry = nullptr;
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
void Application::render(gfx::Frame* frame)
{
	onRender(frame);
	EventDispatcher<AppRenderEvent>::trigger(AppRenderEvent{ frame });
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

Application* Application::app()
{
	return s_app;
}

gfx::GraphicDevice* Application::graphic()
{
	return m_graphic;
}

PlatformDevice* Application::platform()
{
	return m_platform;
}

AudioDevice* Application::audio()
{
	return m_audio;
}

ProgramManager* Application::program()
{
	return m_program;
}

AssetRegistry* Application::resource()
{
	return m_registry;
}

void Application::run(const Config& config)
{
	if (config.app == nullptr)
		throw std::invalid_argument("No app set.");
	
	const Time timestep = Time::milliseconds(10);
	const Time maxUpdate = Time::milliseconds(100);

	Application* app = config.app;
	s_app = app;

	app->create(config);

	gfx::GraphicDevice* graphic = app->m_graphic;
	PlatformDevice* platform = app->m_platform;
	
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
		gfx::Frame* frame = graphic->frame();
		app->frame();
		app->render(frame);
		app->present();
		graphic->present(frame);

		app->end();
		EventDispatcher<QuitEvent>::dispatch();
	} while (app->m_running);

	app->destroy();
}

};