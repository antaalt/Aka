#include "Input.h"

namespace app {
namespace input {

Keyboard keyboard;
Cursor cursor;

void initialize()
{
	for (uint32_t iKey = 0; iKey < g_keyboardKeyCount; iKey++)
	{
		keyboard.pressed[iKey] = false;
		keyboard.timestamp[iKey] = 0;
	}
	for (uint32_t iKey = 0; iKey < g_mouseButtonCount; iKey++)
	{
		cursor.pressed[iKey] = false;
		cursor.timestamp[iKey] = 0;
	}
	cursor.position = { 0.f };
	cursor.delta = { 0.f };
	cursor.scroll = { 0.f };
}

void update()
{
	cursor.delta = { 0.f };
	cursor.scroll = { 0.f };
}

void on_key_down(Key key)
{
	keyboard.pressed[static_cast<int>(key)] = true;
	keyboard.timestamp[static_cast<int>(key)] = 0;
}

void on_key_up(Key key)
{
	keyboard.pressed[static_cast<int>(key)] = false;
	keyboard.timestamp[static_cast<int>(key)] = 0;
}

void on_mouse_button_down(Button button)
{
	cursor.pressed[static_cast<int>(button)] = true;
	cursor.timestamp[static_cast<int>(button)] = 0;
}

void on_mouse_button_up(Button button)
{
	cursor.pressed[static_cast<int>(button)] = false;
	cursor.timestamp[static_cast<int>(button)] = 0;
}

void on_mouse_move(float x, float y)
{
	cursor.delta.x = x - cursor.position.x;
	cursor.delta.y = y - cursor.position.y;
	cursor.position.x = x;
	cursor.position.y = y;
}

void on_mouse_scroll(float x, float y)
{
	cursor.scroll.x = x;
	cursor.scroll.y = y;
}

std::string getKeyName(input::Key key)
{
	// TODO use system function instead ?
	switch (key)
	{
	default: return "Unknown";
	case input::Key::A: return "A";
	case input::Key::B: return "B";
	case input::Key::C: return "C";
	case input::Key::D: return "D";
	case input::Key::E: return "E";
	case input::Key::F: return "F";
	case input::Key::G: return "G";
	case input::Key::H: return "H";
	case input::Key::I: return "I";
	case input::Key::J: return "J";
	case input::Key::K: return "K";
	case input::Key::L: return "L";
	case input::Key::M: return "M";
	case input::Key::N: return "N";
	case input::Key::O: return "O";
	case input::Key::P: return "P";
	case input::Key::Q: return "Q";
	case input::Key::R: return "R";
	case input::Key::S: return "S";
	case input::Key::T: return "T";
	case input::Key::U: return "U";
	case input::Key::V: return "V";
	case input::Key::W: return "W";
	case input::Key::X: return "X";
	case input::Key::Y: return "Y";
	case input::Key::Z: return "Z";
	case input::Key::Num0: return "Num0";
	case input::Key::Num1: return "Num1";
	case input::Key::Num2: return "Num2";
	case input::Key::Num3: return "Num3";
	case input::Key::Num4: return "Num4";
	case input::Key::Num5: return "Num5";
	case input::Key::Num6: return "Num6";
	case input::Key::Num7: return "Num7";
	case input::Key::Num8: return "Num8";
	case input::Key::Num9: return "Num9";
	case input::Key::NumPad0: return "NumPad0";
	case input::Key::NumPad1: return "NumPad1";
	case input::Key::NumPad2: return "NumPad2";
	case input::Key::NumPad3: return "NumPad3";
	case input::Key::NumPad4: return "NumPad4";
	case input::Key::NumPad5: return "NumPad5";
	case input::Key::NumPad6: return "NumPad6";
	case input::Key::NumPad7: return "NumPad7";
	case input::Key::NumPad8: return "NumPad8";
	case input::Key::NumPad9: return "NumPad9";
	case input::Key::RightAlt: return "RightAlt";
	case input::Key::LeftAlt: return "LeftAlt";
	case input::Key::RightCtrl: return "RightCtrl";
	case input::Key::LeftCtrl: return "LeftCtrl";
	case input::Key::RightShift: return "RightShift";
	case input::Key::LeftShift: return "LeftShift";
	case input::Key::Escape: return "Escape";
	case input::Key::Enter: return "Enter";
	case input::Key::Space: return "Space";
	case input::Key::BackSpace: return "BackSpace";
	case input::Key::Tab: return "Tab";
	case input::Key::ArrowLeft: return "ArrowLeft";
	case input::Key::ArrowRight: return "ArrowRight";
	case input::Key::ArrowUp: return "ArrowUp";
	case input::Key::ArrowDown: return "ArrowDown";
	case input::Key::CapsLock: return "CapsLock";
	case input::Key::PageUp: return "PageUp";
	case input::Key::PageDown: return "PageDown";
	case input::Key::PrintScreen: return "PrintScreen";
	}
}

bool pressed(Key key)
{
	return keyboard.pressed[static_cast<int>(key)];
}

bool pressed(Button button)
{
	return cursor.pressed[static_cast<int>(button)];
}

Position& mouse()
{
	return cursor.position;
}

Position& delta()
{
	return cursor.delta;
}

Position& scroll()
{
	return cursor.scroll;
}

};
};
