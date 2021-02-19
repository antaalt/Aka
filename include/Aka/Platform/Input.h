#pragma once

#include <stdint.h>
#include <string>

namespace aka {
namespace input {

enum class KeyboardLayout {
	Azerty,
	Qwerty,
	Qwertz,
	Qzerty,

	Count, // Number of layout

	Unknown,
};

// Key correspond to Qwerty.
enum class Key {
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

enum class Button {
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

std::ostream& operator<<(std::ostream& os, const Key key);
std::ostream& operator<<(std::ostream& os, const KeyboardLayout layout);
std::ostream& operator<<(std::ostream& os, const Button button);

// Counts
constexpr uint32_t getKeyCount() { return static_cast<uint32_t>(Key::Count); }
constexpr uint32_t getButtonCount() { return static_cast<uint32_t>(Button::Count); }

// Structs
struct Keyboard {
	bool pressed[getKeyCount()];
	bool down[getKeyCount()];
	bool up[getKeyCount()];
	uint64_t timestamp[getKeyCount()];
	KeyboardLayout layout;
};

struct Position {
	float x, y;
};

struct Cursor {
	bool pressed[getButtonCount()];
	bool down[getButtonCount()];
	bool up[getButtonCount()];
	uint64_t timestamp[getButtonCount()];
	Position position; // raw position
	Position delta; // relative movement
	Position scroll;
};

// Get the name of a specific key
const char* getKeyName(Key key);
// Get the name of a specific button
const char* getButtonName(Button button);
// Get the name of a specific layout
const char* getKeyboardLayoutName(KeyboardLayout layout);
// Get the currently used layout for the keyboard
KeyboardLayout getKeyboardLayout();

// Actions
bool down(Key key);
bool up(Key key);
bool pressed(Key key);
bool down(Button button);
bool up(Button button);
bool pressed(Button button);
const Position &mouse();
const Position &delta();
const Position &scroll();

};
};

