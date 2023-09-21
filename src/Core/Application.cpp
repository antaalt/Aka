#include <Aka/Core/Application.h>

#include <Aka/Core/Layer.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Audio/AudioDevice.h>
#include <Aka/OS/OS.h>
#include <Aka/OS/Logger.h>

namespace aka {

Application* Application::s_app = nullptr;

Application::Application() :
	m_platform(nullptr),
	m_graphic(nullptr),
	m_audio(nullptr),
	m_program(new ShaderRegistry),
	m_library(new AssetLibrary),
	m_root(new Layer),
	m_needClientResize(false),
	m_width(0),
	m_height(0),
	m_running(true)
{
}
Application::~Application()
{
	delete m_root;
	delete m_library;
	delete m_program;
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
	m_width = config.platform.width;
	m_height = config.platform.height;
	EventDispatcher<AppCreateEvent>::trigger(AppCreateEvent{});
	m_root->create(graphic());
	onCreate(config.argc, config.argv);
}
void Application::destroy()
{
	graphic()->wait();
	EventDispatcher<AppDestroyEvent>::trigger(AppDestroyEvent{});
	m_root->destroy(graphic());
	onDestroy();
	m_program->destroy(m_graphic);
	m_library->destroy(m_graphic);
	AudioDevice::destroy(m_audio);
	gfx::GraphicDevice::destroy(m_graphic);
	PlatformDevice::destroy(m_platform);
	
	m_audio = nullptr;
	m_graphic = nullptr;
	m_platform = nullptr;
}
void Application::start()
{
}
void Application::update(Time deltaTime)
{
	onUpdate(deltaTime);
	m_root->update(deltaTime);
	m_library->update();
	EventDispatcher<AppUpdateEvent>::trigger(AppUpdateEvent{ deltaTime });
}
void Application::fixedUpdate(Time deltaTime)
{
	onFixedUpdate(deltaTime);
	m_root->fixedUpdate(deltaTime);
	EventDispatcher<AppFixedUpdateEvent>::trigger(AppFixedUpdateEvent{ deltaTime });
}
void Application::frame()
{
	onFrame();
	m_root->frame();
	EventDispatcher<AppFrameEvent>::trigger(AppFrameEvent{});
}
void Application::render(gfx::Frame* frame)
{
	onRender(frame);
	m_root->render(frame);
	EventDispatcher<AppRenderEvent>::trigger(AppRenderEvent{ frame });
}
void Application::present()
{
	onPresent();
	m_root->present();
	EventDispatcher<AppPresentEvent>::trigger(AppPresentEvent{});
}
void Application::end()
{
}
void Application::resize()
{
	m_needClientResize = false;
	EventDispatcher<AppResizeEvent>::trigger(AppResizeEvent{ m_width, m_height });
	m_root->resize(m_width, m_height);
	onResize(m_width, m_height);

}
void Application::onReceive(const WindowResizeEvent& event)
{
	// For now, we just ignore this event as window resizing is handle with swapchain recreation.
	// But screen coordinates != pixel coordinates (aka backbuffer size)
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

Layer& Application::getRoot()
{
	return *m_root;
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

ShaderRegistry* Application::program()
{
	return m_program;
}

AssetLibrary* Application::assets()
{
	return m_library;
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
		if (frame != nullptr)
		{
			app->frame();
			app->render(frame);
			app->present();
			gfx::SwapchainStatus status = graphic->present(frame);
			if (status == gfx::SwapchainStatus::Recreated)
			{
				app->m_needClientResize = true;
				graphic->getBackbufferSize(app->m_width, app->m_height);
			}
		}
		else
		{
			app->m_needClientResize = true;
			graphic->getBackbufferSize(app->m_width, app->m_height);
		}

		if (app->m_needClientResize)
		{
			app->resize();
		}

		app->end();
		EventDispatcher<QuitEvent>::dispatch();
	} while (app->m_running);

	app->destroy();
}

};