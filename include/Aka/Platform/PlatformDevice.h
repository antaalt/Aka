#pragma once

#include <Aka/Platform/Platform.h>
#include <Aka/Platform/PlatformWindow.h>
#include <Aka/Platform/Input.h>
#include <Aka/OS/Path.h>
#include <Aka/Core/Encoding.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/Core/Container/HashMap.hpp>

#include <map>
#include <stdint.h>

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

class PlatformDevice
{
public:
	PlatformDevice();
	virtual ~PlatformDevice();

	static PlatformDevice* create();
	static void destroy(PlatformDevice* device);

	virtual void initialize() = 0;
	virtual void shutdown() = 0;

	virtual PlatformWindow* createWindow(const PlatformWindowConfig& cfg) = 0;
	virtual void destroyWindow(PlatformWindow* window) = 0;
};

};