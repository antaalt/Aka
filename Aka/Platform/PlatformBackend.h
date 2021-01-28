#pragma once

#include "../OS/Platform.h"

namespace aka {

struct Config;

class PlatformBackend
{
public:
	static void initialize(const Config& config);
	static void destroy();
	static void frame();
	static void present();
	static bool running();
	static void getSize(uint32_t* width, uint32_t* height);
	static void setSize(uint32_t width, uint32_t height);
	static void setLimits(uint32_t minWidth, uint32_t minHeight, uint32_t maxWidth, uint32_t maxHeight);

	static GLFWwindow* getGLFW3Handle();

#if defined(AKA_USE_D3D11)
	static HWND getD3DHandle();
#endif
};

};