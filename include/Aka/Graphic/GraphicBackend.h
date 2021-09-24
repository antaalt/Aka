#pragma once

#include <Aka/Graphic/GraphicDevice.h>

namespace aka {

/*struct PlatformDevice; // GLFW3, SDL2... (Handle inputs as well)
struct GraphicDevice; // OpenGL, DirectX...
struct AudioDevice; // RtAudio, OpenAL...

enum class DeviceID {};

struct Device 
{
	// Initialize a device instance.
	static void initialize();
	// Destroy all devices
	static void destroy();

	// Get a device that manage rendering API
	static GraphicDevice* graphic();
	// Get a device that manage window API
	static PlatformDevice* platform();
	// Get a device that manage audio API
	static AudioDevice* audio();
};*/

class GraphicBackend {
public:
	// Initialize the graphic API
	static void initialize(uint32_t width, uint32_t height);
	// Destroy the graphic API
	static void destroy();
	// Get the device
	static GraphicDevice* device();
};

}