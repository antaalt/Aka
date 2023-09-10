#pragma once

#include <Aka/Platform/Platform.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Audio/AudioDevice.h>

#include <Aka/Core/Event.h>
#include <Aka/Core/View.h>

#include <Aka/Resource/Shader/Shader.h>
#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Resource/AssetRegistry.h>

namespace aka {

class Application;
class Layer;
class ProgramManager;
class ResourceManager;

struct Config
{
	gfx::GraphicConfig graphic = {};
	AudioConfig audio = {};
	PlatformConfig platform = {};
	Application* app = nullptr;
	Path directory;
	int argc = 0;
	char** argv = nullptr;
};

// Event to notify app to exit.
struct QuitEvent {};

// Event to notify a view change
struct ViewChangedEvent
{
	View::Ptr view;
};

struct AppCreateEvent {

};
struct AppDestroyEvent {

};
struct AppFixedUpdateEvent
{
	Time deltaTime;
};
struct AppUpdateEvent
{
	Time deltaTime;
};
struct AppRenderEvent { gfx::Frame* frame; };
struct AppFrameEvent {};
struct AppPresentEvent {};
struct AppResizeEvent { uint32_t width; uint32_t height; };

class Application : 
	EventListener<QuitEvent>,
	EventListener<WindowResizeEvent>,
	EventListener<BackbufferResizeEvent>
{
public:
	Application();
	virtual ~Application();
private:
	// Create the application and its resources.
	void create(const Config& config);
	// Destroy everything related to the app.
	void destroy();
	// First function called in a loop
	void start();
	// Update the app. deltaTime is time elapsed since last frame
	void update(Time deltaTime);
	// Update the app multiple time per frame at a fixed timestep
	void fixedUpdate(Time deltaTime);
	// Called before render for the app.
	void frame();
	// Render the app.
	void render(gfx::Frame* frame);
	// Called before present of the frame
	void present();
	// Last function called in a loop
	void end();
	// Resize the app
	void resize();
	// Called on app resize
	void onReceive(const WindowResizeEvent& event) override;
	// Called on app backbuffer resize
	void onReceive(const BackbufferResizeEvent& event) override;
	// Called on app quit request
	void onReceive(const QuitEvent& event) override;
protected:
	// Called on app creation.
	virtual void onCreate(int argc, char* argv[]) {}
	// Called on app destruction.
	virtual void onDestroy() {}
	// Called on app update
	virtual void onUpdate(Time deltaTime) {}
	// Called on app update at a fixed timestep
	virtual void onFixedUpdate(Time deltaTime) {}
	// Called before app render
	virtual void onFrame() {}
	// Called on app render
	virtual void onRender(gfx::Frame* frame) {}
	// Called before present of the app
	virtual void onPresent() {}
	// Called on app resize
	virtual void onResize(uint32_t width, uint32_t height) {}
public:
	// Get the current app width
	uint32_t width() const;
	// Get the current app height
	uint32_t height() const;
	// Get the root layer
	Layer& getRoot();
public:
	// Entry point of the application
	static void run(const Config& config);
	// Get the application
	static Application* app();
	// Get the graphic device
	gfx::GraphicDevice* graphic();
	// Get the platform device
	PlatformDevice* platform();
	// Get the audio device
	AudioDevice* audio();
	// Get the program manager
	ShaderRegistry* program();
	// Get the resource manager
	AssetRegistry* resource();
private:
	static Application* s_app;
	PlatformDevice* m_platform;
	gfx::GraphicDevice* m_graphic;
	AudioDevice* m_audio;
	ShaderRegistry* m_program;
	AssetRegistry* m_registry;
private:
	Layer* m_root;
	uint32_t m_width, m_height;
	bool m_needClientResize;
	bool m_running;
};

}