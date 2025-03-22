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
#include <Aka/Scene/Component/ArcballComponent.hpp>
#include <Aka/Scene/Component/RigidBodyComponent.hpp>

namespace aka {

Application* Application::s_app = nullptr;

Application::Application(const Config& config) :
	m_config((OS::setcwd(config.directory), config)), // Some comma operator magic !
	m_platform(PlatformDevice::create()),
	m_window(nullptr), // Deferred creation
	m_physicalDeviceFeatures(config.graphic.features),
	m_windowInitConfig(config.platform),
	m_instance(gfx::Instance::create(config.graphic.api)),
	m_audio(AudioDevice::create(config.audio)),
	m_swapchain(gfx::SwapchainHandle::null),
	m_program(mem::akaNew<ShaderRegistry>(AllocatorMemoryType::Object, AllocatorCategory::Graphic)),
	m_assets(mem::akaNew<AssetLibrary>(AllocatorMemoryType::Object, AllocatorCategory::Assets)),
	m_root(mem::akaNew<Layer>(AllocatorMemoryType::Object, AllocatorCategory::Global)),
	m_running(true),
	m_renderer(mem::akaNew<Renderer>(AllocatorMemoryType::Object, AllocatorCategory::Graphic, m_assets))
{
	AKA_ASSERT(s_app == nullptr, "Application instance already created");
	s_app = this;
	AKA_ASSERT(OS::File::exist(AssetPath("shaders/renderer/asset.glsl", AssetPathType::Common).getAbsolutePath()), "Set your cwd to the root of this project, with Config struct passed to Application constructor in main function.");
}
Application::~Application()
{
	// Destroy all pointers
	m_platform->destroyWindow(m_window);
	m_instance->destroy(m_device);
	AudioDevice::destroy(m_audio);
	gfx::Instance::destroy(m_instance);
	PlatformDevice::destroy(m_platform);
	mem::akaDelete(m_root);
	mem::akaDelete(m_assets);
	mem::akaDelete(m_program);
	mem::akaDelete(m_renderer);
}
bool Application::create()
{
	AKA_ASSERT(m_platform != nullptr, "No platform");
	AKA_ASSERT(m_device != nullptr, "No graphics");
	AKA_ASSERT(m_audio != nullptr, "No audio");
	AKA_ASSERT(m_program != nullptr, "No shaders");
	AKA_ASSERT(m_assets != nullptr, "No assets");
	AKA_ASSERT(m_root != nullptr, "No layers");

	m_platform->initialize();
	m_instance->initialize();
	m_window = m_platform->createWindow(m_windowInitConfig);
	m_window->initialize(m_instance);
	m_device = m_instance->pick(m_physicalDeviceFeatures, m_window->surface());
	if (!m_device->initialize(m_window->surface()))
		return false;
	m_swapchain = m_device->createSwapchain(
		String::format("%sSwapchain", m_windowInitConfig.name.cstr()).cstr(), 
		m_window->surface(), 
		m_windowInitConfig.width, m_windowInitConfig.height, 
		gfx::TextureFormat::BGRA8, 
		gfx::SwapchainMode::Windowed, 
		gfx::SwapchainType::Performance
	);
	m_audio->initialize(m_config.audio);
	m_renderer->create(m_device);
	EventDispatcher<AppCreateEvent>::trigger(AppCreateEvent{});
	m_root->create(renderer());
	onCreate(m_config.argc, m_config.argv);
	return true;
}
void Application::destroy()
{
	graphic()->wait();
	EventDispatcher<AppDestroyEvent>::trigger(AppDestroyEvent{});
	m_root->destroy(renderer());
	onDestroy();
	m_assets->destroy(m_renderer);
	m_renderer->destroy();
	m_program->destroy(m_device);
	m_audio->shutdown();
	m_device->destroy(m_swapchain);
	m_window->shutdown(m_instance);
	m_device->shutdown();
	m_instance->shutdown();
	m_platform->shutdown();

}
void Application::start()
{
}
void Application::update(Time deltaTime)
{
	// Hot reload shaders
#if defined(AKA_SHADER_HOT_RELOAD)
	program()->reloadIfChanged(graphic());
#endif
	// Update app
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
void Application::render(Renderer* _renderer, gfx::FrameHandle frame)
{
	onRender(_renderer, frame);
	m_renderer->render(frame);
	m_root->render(_renderer, frame);
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
void Application::resize(uint32_t width, uint32_t height)
{
	EventDispatcher<AppResizeEvent>::trigger(AppResizeEvent{ width, height });
	m_renderer->resize(width, height);
	m_root->resize(width, height);
	onResize(width, height);

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
	return m_device;
}
gfx::SwapchainHandle Application::swapchain()
{
	return m_swapchain;
}

Renderer* aka::Application::renderer()
{
	return m_renderer;
}

PlatformDevice* Application::platform()
{
	return m_platform;
}
PlatformWindow* Application::window()
{
	return m_window;
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

void Application::run(Application* app)
{
	AKA_ASSERT(app != nullptr, "No app set.");
	
	const Time timestep = Time::milliseconds(10);
	const Time maxUpdate = Time::milliseconds(100);

	// Early exit on failure.
	if (!app->create())
		return;

	gfx::GraphicDevice* graphic = app->m_device;
	Renderer* renderer = app->m_renderer;
	PlatformDevice* platform = app->m_platform;
	PlatformWindow* window = app->m_window;
	gfx::SwapchainHandle swapchain = app->m_swapchain;
	
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
		window->poll();
		app->update(deltaTime);
		// Rendering
		gfx::FrameHandle frame = graphic->frame(swapchain);
		if (frame != gfx::FrameHandle::null)
		{
			app->preRender();
			app->render(renderer, frame);
			app->postRender();
			gfx::SwapchainStatus status = graphic->present(swapchain, frame);
			if (status == gfx::SwapchainStatus::Recreated)
			{
				gfx::SwapchainExtent extent = graphic->getSwapchainExtent(swapchain);
				app->resize(extent.width, extent.height);
			}
		}
		else
		{
			gfx::SwapchainExtent extent = graphic->getSwapchainExtent(swapchain);
			app->resize(extent.width, extent.height);
		}

		app->end();
		EventDispatcher<QuitEvent>::dispatch();
	} while (app->m_running);

	app->destroy();
}

};