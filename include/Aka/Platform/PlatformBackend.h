#pragma once

#include "Platform.h"
#include "../OS/FileSystem.h"

#include <stdint.h>

struct GLFWwindow;
#if defined(AKA_PLATFORM_WINDOWS)
struct HWND__;
typedef HWND__* HWND;
#endif

namespace aka {

struct Config;

class PlatformBackend
{
public:
	static void initialize(const Config& config);
	static void destroy();
	static void update();
	static bool running();
	static void getSize(uint32_t* width, uint32_t* height);
	static void setSize(uint32_t width, uint32_t height);
	static void setLimits(uint32_t minWidth, uint32_t minHeight, uint32_t maxWidth, uint32_t maxHeight);
	static void setFullscreen(bool enabled);
	static void errorDialog(const std::string& message);

#if defined(AKA_PLATFORM_WINDOWS)
	static HWND getWindowsWindowHandle();
#endif
	static GLFWwindow* getGLFW3Handle();
};

};