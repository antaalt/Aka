#pragma once

#include <Aka/Platform/Platform.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Audio/AudioDevice.h>

#include <Aka/Core/Event.h>
#include <Aka/Core/View.h>

namespace aka {

class Application;
class Layer;
class ProgramManager;
class ResourceManager;

struct Config
{
	GraphicConfig graphic = {};
	AudioConfig audio = {};
	PlatformConfig platform = {};
	Application* app = nullptr;
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
struct AppRenderEvent {};
struct AppFrameEvent {};
struct AppPresentEvent {};
struct AppResizeEvent { uint32_t width; uint32_t height; };

class Application : 
	EventListener<QuitEvent>,
	EventListener<WindowResizeEvent>
{
public:
	Application(const std::vector<Layer*> layers);
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
	void render();
	// Called before present of the frame
	void present();
	// Last function called in a loop
	void end();
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
	virtual void onFrame() {}
	// Called on app render
	virtual void onRender() {}
	// Called before present of the app
	virtual void onPresent() {}
	// Called on app resize
	virtual void onResize(uint32_t width, uint32_t height) {}
	// Get the current app width
	uint32_t width() const;
	// Get the current app height
	uint32_t height() const;
public:
	// Entry point of the application
	static void run(const Config& config);
	// Get the graphic device
	static GraphicDevice* graphic();
	// Get the platform device
	static PlatformDevice* platform();
	// Get the audio device
	static AudioDevice* audio();
	// Get the program manager
	static ProgramManager* program();
	// Get the resource manager
	static ResourceManager* resource();
private:
	static PlatformDevice* s_platform;
	static GraphicDevice* s_graphic;
	static AudioDevice* s_audio;
	static ProgramManager* s_program;
	static ResourceManager* s_resource;
private:
	std::vector<Layer*> m_layers;
	uint32_t m_width, m_height;
	bool m_running;
};

}