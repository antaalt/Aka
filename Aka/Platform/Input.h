#pragma once

#include <stdint.h>
#include <string>

namespace aka {
namespace input {

enum class KeyboardLayout {
	Azerty,
	Qwerty,
	Default = Qwerty
};

enum class Key {
	Unknown,
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
	NumPad0,
	NumPad1,
	NumPad2,
	NumPad3,
	NumPad4,
	NumPad5,
	NumPad6,
	NumPad7,
	NumPad8,
	NumPad9,
	Divide,
	Decimal,
	Add,
	Substract,
	Separator,
	Multiply,

	RightAlt,
	LeftAlt,
	RightCtrl,
	LeftCtrl,
	RightShift,
	LeftShift,
	Escape,
	Enter,
	Space,
	BackSpace,
	Tab,
	ArrowLeft,
	ArrowRight,
	ArrowUp,
	ArrowDown,
	CapsLock,
	PageUp,
	PageDown,
	PrintScreen,
	Clear,
	End,
	Home,

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
};

// Counts
const uint32_t g_keyboardKeyCount = 512; // should contain every glfw key code
const uint32_t g_mouseButtonCount = 16; // should contain every glfw mouse button code

// Set inputs
void initialize();
void update();
void on_key_down(Key key);
void on_key_up(Key key);
void on_mouse_button_down(Button button);
void on_mouse_button_up(Button button);
void on_mouse_move(float x, float y);
void on_mouse_scroll(float x, float y);

// Structs
struct Keyboard {
	bool pressed[g_keyboardKeyCount];
	bool down[g_keyboardKeyCount];
	bool up[g_keyboardKeyCount];
	uint64_t timestamp[g_keyboardKeyCount];
	KeyboardLayout layout;
};

struct Position {
	float x, y;
};

struct Cursor {
	bool pressed[g_mouseButtonCount];
	bool down[g_mouseButtonCount];
	bool up[g_mouseButtonCount];
	uint64_t timestamp[g_keyboardKeyCount];
	Position position; // raw position
	Position delta; // relative movement
	Position scroll;
};

// Infos
std::string getKeyName(input::Key key);

// Actions
bool down(Key key);
bool up(Key key);
bool pressed(Key key);
bool down(Button button);
bool up(Button button);
bool pressed(Button button);
Position &mouse();
Position &delta();
Position &scroll();

};
};

