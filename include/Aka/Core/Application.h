#pragma once

#include <Aka/Platform/Platform.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Audio/AudioDevice.h>

#include <Aka/Core/Event.h>

#include <Aka/Resource/Shader/Shader.h>
#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

class Application;
class Layer;

struct Config
{
	gfx::GraphicConfig graphic = {};
	AudioConfig audio = {};
	PlatformWindowConfig platform = {};
	Path directory = "../../../"; // Default build path for CMake relative to project
	int argc = 0;
	char** argv = nullptr;
};

// Event to notify app to exit.
struct QuitEvent {};

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
struct AppRenderEvent { gfx::FrameHandle frame; };
struct AppFrameEvent {};
struct AppPresentEvent {};
struct AppResizeEvent { uint32_t width; uint32_t height; };

class Application : 
	EventListener<QuitEvent>,
	EventListener<WindowResizeEvent>
{
public:
	Application(const Config& cfg);
	Application(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(const Application&) = delete;
	Application& operator=(Application&&) = delete;
	virtual ~Application();
private:
	// Create the application and its resources.
	bool create();
	// Destroy everything related to the app.
	void destroy();
	// First function called in a loop
	void start();
	// Update the app. deltaTime is time elapsed since last frame
	void update(Time deltaTime);
	// Update the app multiple time per frame at a fixed timestep
	void fixedUpdate(Time deltaTime);
	// Called before render for the app, after frame acquisition
	void preRender();
	// Render the app.
	void render(Renderer* _renderer, gfx::FrameHandle frame);
	// Called after render, before present of the frame
	void postRender();
	// Last function called in a loop
	void end();
	// Resize the app
	void resize(uint32_t width, uint32_t height);
	// Called on app resize
	void onReceive(const WindowResizeEvent& event) override;
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
	virtual void onPreRender() {}
	// Called on app render
	virtual void onRender(Renderer* _renderer, gfx::FrameHandle frame) {}
	// Called before present of the app
	virtual void onPostRender() {}
	// Called on app resize
	virtual void onResize(uint32_t width, uint32_t height) {}
public:
	// Get the root layer
	Layer& getRoot();
public:
	// Entry point of the application
	static void run(Application* app);
	// Get the application
	static Application* app();
	// Get the graphic device
	gfx::GraphicDevice* graphic();
	// Get the renderer
	Renderer* renderer();
	// Get the platform device
	PlatformDevice* platform();
	// Get the main window device
	PlatformWindow* window();
	// Get the audio device
	AudioDevice* audio();
	// Get the program manager
	ShaderRegistry* program();
	// Get the asset manager
	AssetLibrary* assets();
private:
	static Application* s_app;
	Config m_config;
	PlatformDevice* m_platform;
	PlatformWindow* m_window;
	PlatformWindowConfig m_windowInitConfig;
	gfx::GraphicDevice* m_graphic;
	AudioDevice* m_audio;
	ShaderRegistry* m_program;
	AssetLibrary* m_assets;
	Renderer* m_renderer;
private:
	Layer* m_root;
	bool m_running;
};

}