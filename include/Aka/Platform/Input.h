#pragma once

#include <stdint.h>
#include <string>

namespace aka {

enum class KeyboardLayout {
	Azerty,
	Qwerty,
	Qwertz,
	Qzerty,

	Count, // Number of layout

	Unknown,
};

// Key correspond to Qwerty.
enum class KeyboardKey {
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

enum class MouseButton {
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

enum class GamepadButton {
	A,
	B,
	X,
	Y,
	LeftStick,
	RightStick,
	Back,
	Start,
	Guide,
	LeftThumb,
	RightThumb,
	Up,
	Right,
	Left,
	Down,

	Count,

	Cross = A,
	Circle = B,
	Square = X,
	Triangle = Y
};

enum class GamepadAxis {
	Left,
	Right,
	TriggerLeft,
	TriggerRight,

	Count
};

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
struct Position {
	float x, y;
};

struct Keyboard {
	// Is key just pressed
	static bool down(KeyboardKey key);
	// Is key just released
	static bool up(KeyboardKey key);
	// Is key pressed
	static bool pressed(KeyboardKey key);
	// Is any key pressed
	static bool any();
	// Get the name of a specific key
	static const char* name(KeyboardKey key);
	// Get the name of a specific layout
	static const char* layoutName(KeyboardLayout layout);
	// Get the currently used layout for the keyboard
	static KeyboardLayout layout();

	// Update the mouse values.
	void update();

	bool _pressed[getKeyboardKeyCount()];
	bool _down[getKeyboardKeyCount()];
	bool _up[getKeyboardKeyCount()];
	uint64_t _timestamp[getKeyboardKeyCount()];
	KeyboardLayout _layout;
	size_t _pressedCount;
};

struct Mouse {
	// Is mouse button just pressed
	static bool down(MouseButton button);
	// Is mouse button just released
	static bool up(MouseButton button);
	// Is mouse button pressed
	static bool pressed(MouseButton button);
	// Is mouse in window
	static bool focused();
	// Get the name of a specific button
	static const char* name(MouseButton button);
	// Get the current mouse position
	static const Position& position();
	// Get the current mouse delta since last frame
	static const Position& delta();
	// Get the mouse current scroll value
	static const Position& scroll();

	// Update the mouse values.
	void update();

	bool _pressed[getMouseButtonCount()];
	bool _down[getMouseButtonCount()];
	bool _up[getMouseButtonCount()];
	uint64_t _timestamp[getMouseButtonCount()];
	Position _position; // raw position
	Position _delta; // relative movement
	Position _scroll;
	bool _focus;
};

enum class GamepadID : unsigned int {};

struct Gamepad {
	// Get the first gamepad
	static GamepadID get();
	// Is gamepad connected
	static bool connected(GamepadID gid);
	// Get gamepad name
	static const char* name(GamepadID gid);
	// Is gamepad button just pressed
	static bool down(GamepadID gid, GamepadButton button);
	// Is gamepad button just released
	static bool up(GamepadID gid, GamepadButton button);
	// Is gamepad button pressed
	static bool pressed(GamepadID gid, GamepadButton button);
	// Get the axis value of a specific gamepad axis
	static const Position& axis(GamepadID gid, GamepadAxis axis);
	// Get the name of a specific gamepad button
	static const char* name(GamepadButton button);
	// Get the name of a specific gamepad axis
	static const char* name(GamepadAxis axis);

	// Update the gamepad values
	void update();

	const char* _name;
	bool _pressed[getGamepadButtonCount()];
	bool _down[getGamepadButtonCount()];
	bool _up[getGamepadButtonCount()];
	uint64_t _timestamp[getGamepadButtonCount()];
	Position _axes[getGamepadAxisCount()];
};

// Events
struct KeyboardKeyDownEvent {
	KeyboardKey key;
};
struct KeyboardKeyUpEvent {
	KeyboardKey key;
};
struct KeyboardKeyRepeatEvent {
	KeyboardKey key;
};
struct MouseButtonDownEvent {
	MouseButton button;
};
struct MouseButtonUpEvent {
	MouseButton button;
};
struct MouseButtonRepeatEvent {
	MouseButton button;
};
struct MouseMoveEvent {
	float x, y;
};
struct MouseScrollEvent {
	float x, y;
};
struct MouseEnterEvent {
	// emtpy !
};
struct MouseLeaveEvent {
	// emtpy !
};
struct GamepadConnectedEvent {
	GamepadID id;
	const char* name;
};
struct GamepadDisconnectedEvent {
	GamepadID id;
};
struct GamepadButtonDownEvent {
	GamepadID id;
	GamepadButton button;
};
struct GamepadButtonUpEvent {
	GamepadID id;
	GamepadButton button;
};
struct GamepadAxesMotionEvent {
	GamepadID id;
	GamepadAxis axis;
	Position value;
};

};

