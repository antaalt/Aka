#pragma once

#include "Platform.h"

namespace app {
namespace input {

// Enum map
enum class Key {
	UNKNOWN = GLFW_KEY_UNKNOWN,
	A = GLFW_KEY_A,
	B = GLFW_KEY_B,
	C = GLFW_KEY_C,
	D = GLFW_KEY_D,
	E = GLFW_KEY_E,
	F = GLFW_KEY_F,
	G = GLFW_KEY_G,
	H = GLFW_KEY_H,
	I = GLFW_KEY_I,
	J = GLFW_KEY_J,
	K = GLFW_KEY_K,
	L = GLFW_KEY_L,
	M = GLFW_KEY_M,
	N = GLFW_KEY_N,
	O = GLFW_KEY_O,
	P = GLFW_KEY_P,
	Q = GLFW_KEY_Q,
	R = GLFW_KEY_R,
	S = GLFW_KEY_S,
	T = GLFW_KEY_T,
	U = GLFW_KEY_U,
	V = GLFW_KEY_V,
	W = GLFW_KEY_W,
	X = GLFW_KEY_X,
	Y = GLFW_KEY_Y,
	Z = GLFW_KEY_Z,
	RightAlt = GLFW_KEY_RIGHT_ALT,
	LeftAlt = GLFW_KEY_LEFT_ALT,
	RightCtrl = GLFW_KEY_RIGHT_CONTROL,
	LeftCtrl = GLFW_KEY_LEFT_CONTROL,
	Escape = GLFW_KEY_ESCAPE,
	Enter = GLFW_KEY_ENTER,
	Space = GLFW_KEY_SPACE,
	BackSpace = GLFW_KEY_BACKSPACE,
	Tab = GLFW_KEY_TAB,
};

enum class Button {
	Button1 = GLFW_MOUSE_BUTTON_1,
	Button2 = GLFW_MOUSE_BUTTON_2,
	Button3 = GLFW_MOUSE_BUTTON_3,
	Button4 = GLFW_MOUSE_BUTTON_4,
	Button5 = GLFW_MOUSE_BUTTON_5,
	Button6 = GLFW_MOUSE_BUTTON_6,
	Button7 = GLFW_MOUSE_BUTTON_7,
	Button8 = GLFW_MOUSE_BUTTON_8,
};

// Counts
const uint32_t g_keyboardKeyCount = 512; // should contain every glfw key code
const uint32_t g_mouseButtonCount = 16; // should contain every glfw mouse button code

// Set inputs
void initialize();
void on_key_down(Key key);
void on_key_up(Key key);
void on_mouse_button_down(Button button);
void on_mouse_button_up(Button button);
void on_mouse_move(float x, float y);
void on_mouse_scroll(float x, float y);

// Structs
struct Keyboard {
	bool pressed[g_keyboardKeyCount];
	uint64_t timestamp[g_keyboardKeyCount];
};

struct Cursor {
	bool pressed[g_mouseButtonCount];
	uint64_t timestamp[g_keyboardKeyCount];
	struct {
		float x, y;
	} position;
	struct {
		float x, y;
	} scroll;
};

// Actions
bool pressed(Key key);
bool pressed(Button button);

};
};

