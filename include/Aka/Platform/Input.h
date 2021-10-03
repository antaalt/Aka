#pragma once

#include <stdint.h>
#include <string>

namespace aka {

enum class KeyboardLayout
{
	Azerty,
	Qwerty,
	Qwertz,
	Qzerty,

	Count, // Number of layout

	Unknown,
};

// Key correspond to Qwerty.
enum class KeyboardKey 
{
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,

	Num0,
	Num1,
	Num2,
	Num3,
	Num4,
	Num5,
	Num6,
	Num7,
	Num8,
	Num9,

	NumLock,
	KeyPad0,
	KeyPad1,
	KeyPad2,
	KeyPad3,
	KeyPad4,
	KeyPad5,
	KeyPad6,
	KeyPad7,
	KeyPad8,
	KeyPad9,
	KeyPadDivide,
	KeyPadDecimal,
	KeyPadAdd,
	KeyPadSubstract,
	KeyPadMultiply,
	KeyPadEnter,

	AltRight,
	AltLeft,
	ControlRight,
	ControlLeft,
	ShiftRight,
	ShiftLeft,

	ArrowLeft,
	ArrowRight,
	ArrowUp,
	ArrowDown,

	Escape,
	Enter,
	Space,
	BackSpace,
	Tab,
	CapsLock,
	PageUp,
	PageDown,
	PrintScreen,
	Pause,
	Menu,
	Insert,
	Delete,
	Help,
	Clear,
	ScrollLock,
	End,
	Home,

	Slash,
	BackSlash,
	Plus,
	Minus,
	Equal,
	Comma,
	Period,
	Semicolon,
	Apostrophe,
	Grave,
	BracketAngle,
	BracketLeft,
	BracketRight,

	AltPrintScreen,

	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,

	Count, // Number of keys

	Unknown,
};

enum class MouseButton 
{
	Button1,
	Button2,
	Button3,
	Button4,
	Button5,
	Button6,
	Button7,
	Button8,

	Count, // Number of buttons

	Unknown,

	ButtonLeft = Button1,
	ButtonRight = Button2,
	ButtonMiddle = Button3,
};

enum class GamepadButton 
{
	A,
	B,
	X,
	Y,
	LeftBumper,
	RightBumper,
	Select,
	Start,
	Home,
	LeftStick,
	RightStick,
	DpadUp,
	DpadRight,
	DpadLeft,
	DpadDown,

	Count,

	Cross = A,
	Circle = B,
	Square = X,
	Triangle = Y
};

enum class GamepadAxis 
{
	Left,
	Right,
	TriggerLeft,
	TriggerRight,

	Count
};

// Logs
std::ostream& operator<<(std::ostream& os, const KeyboardKey key);
std::ostream& operator<<(std::ostream& os, const KeyboardLayout layout);
std::ostream& operator<<(std::ostream& os, const MouseButton button);
std::ostream& operator<<(std::ostream& os, const GamepadButton button);
std::ostream& operator<<(std::ostream& os, const GamepadAxis axis);

// Counts
constexpr uint32_t getKeyboardKeyCount() { return static_cast<uint32_t>(KeyboardKey::Count); }
constexpr uint32_t getMouseButtonCount() { return static_cast<uint32_t>(MouseButton::Count); }
constexpr uint32_t getGamepadButtonCount() { return static_cast<uint32_t>(GamepadButton::Count); }
constexpr uint32_t getGamepadAxisCount() { return static_cast<uint32_t>(GamepadAxis::Count); }

// Structs
struct Position 
{
	float x, y;
};

struct Keyboard 
{
	friend class PlatformDevice;
	// Is key just pressed
	bool down(KeyboardKey key) const;
	// Is key just released
	bool up(KeyboardKey key) const;
	// Is key pressed
	bool pressed(KeyboardKey key) const;
	// Is any key pressed
	bool any() const;
	// Get the currently used layout for the keyboard
	KeyboardLayout layout() const;

	// Get the name of a specific key
	static const char* name(KeyboardKey key);
	// Get the name of a specific layout
	static const char* layoutName(KeyboardLayout layout);

protected:
	bool m_pressed[getKeyboardKeyCount()];
	bool m_down[getKeyboardKeyCount()];
	bool m_up[getKeyboardKeyCount()];
	KeyboardLayout m_layout;
	size_t m_pressedCount;
};

struct Mouse
{
	friend class PlatformDevice;
	// Is mouse button just pressed
	bool down(MouseButton button) const;
	// Is mouse button just released
	bool up(MouseButton button) const;
	// Is mouse button pressed
	bool pressed(MouseButton button) const;
	// Is mouse in window
	bool focused() const;
	// Get the current mouse position
	const Position& position() const;
	// Get the current mouse delta since last frame
	const Position& delta() const;
	// Get the mouse current scroll value
	const Position& scroll() const;

	// Get the name of a specific button
	static const char* name(MouseButton button);

protected:
	bool m_pressed[getMouseButtonCount()];
	bool m_down[getMouseButtonCount()];
	bool m_up[getMouseButtonCount()];
	Position m_position; // raw position
	Position m_delta; // relative movement
	Position m_scroll;
	bool m_focus;
};

enum class GamepadID : unsigned int {};

struct Gamepad
{
	friend class PlatformDevice;
	// Get gamepad name
	const char* name(GamepadID gid) const;
	// Is gamepad button just pressed
	bool down(GamepadID gid, GamepadButton button) const;
	// Is gamepad button just released
	bool up(GamepadID gid, GamepadButton button) const;
	// Is gamepad button pressed
	bool pressed(GamepadID gid, GamepadButton button) const;
	// Get the axis value of a specific gamepad axis
	const Position& axis(GamepadID gid, GamepadAxis axis) const;

	// Get the name of a specific gamepad button
	static const char* name(GamepadButton button);
	// Get the name of a specific gamepad axis
	static const char* name(GamepadAxis axis);

protected:
	const char* m_name;
	bool m_pressed[getGamepadButtonCount()];
	bool m_down[getGamepadButtonCount()];
	bool m_up[getGamepadButtonCount()];
	Position m_axes[getGamepadAxisCount()];
};

// Events
struct KeyboardKeyDownEvent
{
	KeyboardKey key;
};
struct KeyboardKeyUpEvent 
{
	KeyboardKey key;
};
struct KeyboardKeyRepeatEvent 
{
	KeyboardKey key;
};
struct MouseButtonDownEvent 
{
	MouseButton button;
};
struct MouseButtonUpEvent 
{
	MouseButton button;
};
struct MouseButtonRepeatEvent 
{
	MouseButton button;
};
struct MouseMotionEvent 
{
	float x, y;
};
struct MouseScrollEvent 
{
	float x, y;
};
struct MouseEnterEvent 
{
	// emtpy !
};
struct MouseLeaveEvent 
{
	// emtpy !
};
struct GamepadConnectedEvent 
{
	GamepadID id;
	const char* name;
};
struct GamepadDisconnectedEvent
{
	GamepadID id;
};
struct GamepadButtonDownEvent 
{
	GamepadID id;
	GamepadButton button;
};
struct GamepadButtonUpEvent
{
	GamepadID id;
	GamepadButton button;
};
struct GamepadAxesMotionEvent 
{
	GamepadID id;
	GamepadAxis axis;
	Position value;
};

};

