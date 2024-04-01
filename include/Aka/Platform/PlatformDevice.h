#pragma once

#include <Aka/Platform/Platform.h>
#include <Aka/Platform/Input.h>
#include <Aka/OS/Path.h>
#include <Aka/Core/Encoding.h>
#include <Aka/Core/Container/Vector.h>

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

enum class PlatformFlag
{
	None,
	FullScreen = (1 << 0), // Is window fullscreen
	Resizable = (1 << 1), // Is window resizable
	Decorated = (1 << 2), // Is the window decorated
	Maximized = (1 << 3), // Maximized window on creation
	Transparent = (1 << 4), // Is the backbuffer transparent
	Default = Resizable | Decorated
};

// 3 components squared image.
struct PlatformIcon
{
	byte_t* bytes;
	uint32_t size;
};

struct PlatformConfig
{
	int32_t x = 0;
	int32_t y = 0;
	uint32_t width = 1280;
	uint32_t height = 720;
	String name = "Aka";
	PlatformFlag flags = PlatformFlag::Default;
	PlatformIcon icon = { nullptr, 0 };
};

PlatformFlag operator|(const PlatformFlag& lhs, const PlatformFlag& rhs);
PlatformFlag operator&(const PlatformFlag& lhs, const PlatformFlag& rhs);
PlatformFlag operator~(const PlatformFlag& flag);

class PlatformDevice
{
public:
	PlatformDevice(const PlatformConfig& config);
	virtual ~PlatformDevice();

	static PlatformDevice* create(const PlatformConfig& config);
	static void destroy(PlatformDevice* device);

	virtual void initialize(const PlatformConfig& config) = 0;
	virtual void shutdown() = 0;

	// Get the width of the window
	uint32_t width() const;
	// Get the height of the window
	uint32_t height() const;
	// Get the x position of the window on the screen
	int32_t x() const;
	// Get the y position of the window on the screen
	int32_t y() const;
	// Get the flags of the window
	PlatformFlag flags() const;

	// Get the mouse
	const Mouse& mouse() const;
	// Get the keyboard
	const Keyboard& keyboard() const;
	// Get the gamepad
	const Gamepad& gamepad(GamepadID id) const;
	// Check if gamepad is connected
	bool connected(GamepadID id) const;

	// Poll all the events of the window.
	virtual void poll() = 0;
	// Move the window
	virtual void move(int32_t x, int32_t y) = 0;
	// Resize the window
	virtual void resize(uint32_t width, uint32_t height) = 0;
	// Set the constraints of the window
	virtual void setLimits(uint32_t minWidth, uint32_t minHeight, uint32_t maxWidth, uint32_t maxHeight) = 0;
	// Set fullscreen mode of the window
	virtual void fullscreen(bool enabled) = 0;
	// Get native handle to the window
	virtual void* getNativeHandle() = 0;
protected: // Inputs
	void onInputsUpdate();
	void onKeyboardKeyDown(KeyboardKey key);
	void onKeyboardKeyUp(KeyboardKey key);
	void onMouseButtonDown(MouseButton button);
	void onMouseButtonUp(MouseButton button);
	void onMouseMotion(float x, float y);
	void onMouseScroll(float x, float y);
	void onMouseEnter();
	void onMouseLeave();
	void onGamepadConnected(GamepadID gid, const char* name);
	void onGamepadDisconnected(GamepadID gid);
	void onGamepadButtonDown(GamepadID gid, GamepadButton button);
	void onGamepadButtonUp(GamepadID gid, GamepadButton button);
	void onGamepadAxisMotion(GamepadID gid, GamepadAxis axis, const Position& value);
protected:
	uint32_t m_width;
	uint32_t m_height;
	int32_t m_x;
	int32_t m_y;
	PlatformFlag m_flags;
	Keyboard m_keyboard;
	Mouse m_mouse;
	std::map<GamepadID, Gamepad> m_gamepads;
};

};