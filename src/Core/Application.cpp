#include <Aka/Core/Application.h>

#include <Aka/Core/Layer.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Audio/AudioDevice.h>
#include <Aka/OS/OS.h>
#include <Aka/OS/Logger.h>
#include <Aka/Renderer/Renderer.hpp>
#include <Aka/Scene/Component/StaticMeshComponent.hpp>
#include <Aka/Scene/Component/SkeletalMeshComponent.hpp>
#include <Aka/Scene/Component/CameraComponent.hpp>

namespace aka {

Application* Application::s_app = nullptr;

Application::Application(const Config& config) :
	m_platform(PlatformDevice::create(config.platform)),
	m_graphic(gfx::GraphicDevice::create(config.graphic.api)),
	m_audio(AudioDevice::create(config.audio)),
	m_program(new ShaderRegistry),
	m_assets(new AssetLibrary),
	m_root(new Layer),
	m_needClientResize(false),
	m_width(0),
	m_height(0),
	m_running(true),
	m_renderer(new Renderer(m_graphic, m_assets))
{
	// Register all used components
	AKA_REGISTER_COMPONENT(StaticMeshComponent);
	AKA_REGISTER_COMPONENT(SkeletalMeshComponent);
	AKA_REGISTER_COMPONENT(CameraComponent);
}
Application::~Application()
{
	// Unregister all used components
	AKA_UNREGISTER_COMPONENT(StaticMeshComponent);
	AKA_UNREGISTER_COMPONENT(SkeletalMeshComponent);
	AKA_UNREGISTER_COMPONENT(CameraComponent);
	// Destroy all pointers
	AudioDevice::destroy(m_audio);
	gfx::GraphicDevice::destroy(m_graphic);
	PlatformDevice::destroy(m_platform);
	delete m_root;
	delete m_assets;
	delete m_program;
}
void Application::create(const Config& config)
{
	OS::setcwd(config.directory);
	AKA_ASSERT(m_platform != nullptr, "No platform");
	AKA_ASSERT(m_graphic != nullptr, "No graphics");
	AKA_ASSERT(m_audio != nullptr, "No audio");
	AKA_ASSERT(m_program != nullptr, "No shaders");
	AKA_ASSERT(m_assets != nullptr, "No assets");
	AKA_ASSERT(m_root != nullptr, "No layers");

	m_platform->initialize(config.platform);
	m_graphic->initialize(m_platform, config.graphic);
	m_audio->initialize(config.audio);
	m_renderer->create();
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
	m_assets->destroy(m_renderer);
	m_renderer->destroy();
	m_program->destroy(m_graphic);
	m_audio->shutdown();
	m_graphic->shutdown();
	m_platform->shutdown();

}
void Application::start()
{
}
void Application::update(Time deltaTime)
{
	onUpdate(deltaTime);
	m_root->update(deltaTime);
	m_assets->update();
	EventDispatcher<AppUpdateEvent>::trigger(AppUpdateEvent{ deltaTime });
}
void Application::fixedUpdate(Time deltaTime)
{
	onFixedUpdate(deltaTime);
	m_root->fixedUpdate(deltaTime);
	EventDispatcher<AppFixedUpdateEvent>::trigger(AppFixedUpdateEvent{ deltaTime });
}
void Application::preRender()
{
	onPreRender();
	m_root->preRender();
	EventDispatcher<AppFrameEvent>::trigger(AppFrameEvent{});
}
void Application::render(gfx::GraphicDevice* _device, gfx::FrameHandle frame)
{
	onRender(_device, frame);
	m_renderer->render(frame);
	m_root->render(_device, frame);
	EventDispatcher<AppRenderEvent>::trigger(AppRenderEvent{ frame });
}
void Application::postRender()
{
	onPostRender();
	m_root->postRender();
	EventDispatcher<AppPresentEvent>::trigger(AppPresentEvent{});
}
void Application::end()
{
}
void Application::resize()
{
	m_needClientResize = false;
	EventDispatcher<AppResizeEvent>::trigger(AppResizeEvent{ m_width, m_height });
	m_renderer->resize(m_width, m_height);
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

Renderer* aka::Application::renderer()
{
	return m_renderer;
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
	return m_assets;
}

void Application::run(Application* app, const Config& config)
{
	if (app == nullptr)
		throw std::invalid_argument("No app set.");
	
	const Time timestep = Time::milliseconds(10);
	const Time maxUpdate = Time::milliseconds(100);

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
		gfx::FrameHandle frame = graphic->frame();
		if (frame != gfx::FrameHandle::null)
		{
			app->preRender();
			app->render(graphic, frame);
			app->postRender();
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