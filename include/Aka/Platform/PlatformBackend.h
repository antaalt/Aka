#pragma once

#include <Aka/Platform/Platform.h>
#include <Aka/OS/Path.h>
#include <Aka/Core/Encoding.h>
#include <Aka/Core/Container/Vector.h>

#include <stdint.h>

struct GLFWwindow;

namespace aka {

struct WindowResizeEvent {
	uint32_t width, height;
};
struct BackbufferResizeEvent {
	uint32_t width, height;
};
struct WindowMaximizedEvent {
	bool maximized;
};
struct WindowContentScaledEvent {
	float x, y;
};
struct WindowIconifiedEvent {
	bool iconified;
};
struct WindowFocusedEvent {
	bool focus;
};
struct WindowMovedEvent {
	int x, y;
};
struct WindowRefreshedEvent {
	// empty
};
struct WindowUnicodeCharEvent {
	encoding::CodePoint codepoint;
};
struct WindowDropEvent {
	Vector<Path> paths;
};
struct MonitorConnectedEvent {
	// TODO retrieve monitor data
};
struct MonitorDisconnectedEvent {
	// TODO retrieve monitor data
};

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

	static GLFWwindow* getGLFW3Handle();
};

};