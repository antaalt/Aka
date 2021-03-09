#include <Aka/Platform/Input.h>

#include <Aka/Platform/InputBackend.h>
#include <Aka/OS/Time.h>
#include <Aka/OS/Logger.h>
#include <Aka/Core/Event.h>

#include <iostream>

namespace aka {
namespace input {

static Keyboard keyboard = {};
static Cursor cursor = {};

std::ostream& operator<<(std::ostream& os, const Key key)
{
	return (os << getKeyName(key));
}

std::ostream& operator<<(std::ostream& os, const KeyboardLayout layout)
{
	return (os << getKeyboardLayoutName(layout));
}

std::ostream& operator<<(std::ostream& os, const Button button)
{
	return (os << getButtonName(button));
}

const char* getKeyName(Key key)
{
	switch (key)
	{
	default: return "Unknown";
	case Key::A: return "A";
	case Key::B: return "B";
	case Key::C: return "C";
	case Key::D: return "D";
	case Key::E: return "E";
	case Key::F: return "F";
	case Key::G: return "G";
	case Key::H: return "H";
	case Key::I: return "I";
	case Key::J: return "J";
	case Key::K: return "K";
	case Key::L: return "L";
	case Key::M: return "M";
	case Key::N: return "N";
	case Key::O: return "O";
	case Key::P: return "P";
	case Key::Q: return "Q";
	case Key::R: return "R";
	case Key::S: return "S";
	case Key::T: return "T";
	case Key::U: return "U";
	case Key::V: return "V";
	case Key::W: return "W";
	case Key::X: return "X";
	case Key::Y: return "Y";
	case Key::Z: return "Z";
	case Key::Num0: return "Num0";
	case Key::Num1: return "Num1";
	case Key::Num2: return "Num2";
	case Key::Num3: return "Num3";
	case Key::Num4: return "Num4";
	case Key::Num5: return "Num5";
	case Key::Num6: return "Num6";
	case Key::Num7: return "Num7";
	case Key::Num8: return "Num8";
	case Key::Num9: return "Num9";
	case Key::NumLock: return "NumLock";
	case Key::KeyPad0: return "KeyPad0";
	case Key::KeyPad1: return "KeyPad1";
	case Key::KeyPad2: return "KeyPad2";
	case Key::KeyPad3: return "KeyPad3";
	case Key::KeyPad4: return "KeyPad4";
	case Key::KeyPad5: return "KeyPad5";
	case Key::KeyPad6: return "KeyPad6";
	case Key::KeyPad7: return "KeyPad7";
	case Key::KeyPad8: return "KeyPad8";
	case Key::KeyPad9: return "KeyPad9";
	case Key::KeyPadDivide: return "KeyPadDivide";
	case Key::KeyPadDecimal: return "KeyPadDecimal";
	case Key::KeyPadAdd: return "KeyPadAdd";
	case Key::KeyPadSubstract: return "KeyPadSubstract";
	case Key::KeyPadMultiply: return "KeyPadMultiply";
	case Key::KeyPadEnter: return "KeyPadEnter";
	case Key::AltRight: return "AltRight";
	case Key::AltLeft: return "AltLeft";
	case Key::ControlRight: return "ControlRight";
	case Key::ControlLeft: return "ControlLeft";
	case Key::ShiftRight: return "ShiftRight";
	case Key::ShiftLeft: return "ShiftLeft";
	case Key::Escape: return "Escape";
	case Key::Enter: return "Enter";
	case Key::Space: return "Space";
	case Key::BackSpace: return "BackSpace";
	case Key::Tab: return "Tab";
	case Key::ArrowLeft: return "ArrowLeft";
	case Key::ArrowRight: return "ArrowRight";
	case Key::ArrowUp: return "ArrowUp";
	case Key::ArrowDown: return "ArrowDown";
	case Key::CapsLock: return "CapsLock";
	case Key::PageUp: return "PageUp";
	case Key::PageDown: return "PageDown";
	case Key::Pause: return "Pause";
	case Key::Menu: return "Menu";
	case Key::PrintScreen: return "PrintScreen";
	case Key::Insert: return "Insert";
	case Key::Delete: return "Delete";
	case Key::Help: return "Help";
	case Key::Clear: return "Clear";
	case Key::ScrollLock: return "ScrollLock";
	case Key::End: return "End";
	case Key::Home: return "Home";
	case Key::Slash: return "Slash";
	case Key::BackSlash: return "BackSlash";
	case Key::Plus: return "Plus";
	case Key::Minus: return "Minus";
	case Key::Equal: return "Equal";
	case Key::Comma: return "Comma";
	case Key::Period: return "Period";
	case Key::Semicolon: return "Semicolon";
	case Key::Apostrophe: return "Apostrophe";
	case Key::Grave: return "Grave";
	case Key::BracketAngle: return "BracketAngle";
	case Key::BracketLeft: return "BracketLeft";
	case Key::BracketRight: return "BracketRight";
	case Key::AltPrintScreen: return "AltPrintScreen";
	case Key::F1: return "F1";
	case Key::F2: return "F2";
	case Key::F3: return "F3";
	case Key::F4: return "F4";
	case Key::F5: return "F5";
	case Key::F6: return "F6";
	case Key::F7: return "F7";
	case Key::F8: return "F8";
	case Key::F9: return "F9";
	case Key::F10: return "F10";
	case Key::F11: return "F11";
	case Key::F12: return "F12";
	}
}

const char* getButtonName(Button button)
{
	switch (button)
	{
	default: return "Unknown";
	case Button::Button1: return "ButtonLeft";
	case Button::Button2: return "ButtonRight";
	case Button::Button3: return "ButtonMiddle";
	case Button::Button4: return "Button4";
	case Button::Button5: return "Button5";
	case Button::Button6: return "Button6";
	case Button::Button7: return "Button7";
	case Button::Button8: return "Button8";
	}
}

const char* getKeyboardLayoutName(KeyboardLayout layout)
{
	switch (layout)
	{
	default: return "Unknown";
	case KeyboardLayout::Azerty: return "Azerty";
	case KeyboardLayout::Qwerty: return "Qwerty";
	case KeyboardLayout::Qwertz: return "Qwertz";
	case KeyboardLayout::Qzerty: return "Qzerty";
	}
}

KeyboardLayout getKeyboardLayout()
{
	return keyboard.layout;
}

bool down(Key key)
{
	return keyboard.down[static_cast<int>(key)];
}

bool up(Key key)
{
	return keyboard.up[static_cast<int>(key)];
}

bool pressed(Key key)
{
	return keyboard.pressed[static_cast<int>(key)];
}

bool anyPressed()
{
	return keyboard.pressedCount > 0;
}

bool down(Button button)
{
	return cursor.down[static_cast<int>(button)];
}

bool up(Button button)
{
	return cursor.up[static_cast<int>(button)];
}

bool pressed(Button button)
{
	return cursor.pressed[static_cast<int>(button)];
}

const Position& mouse()
{
	return cursor.position;
}

const Position& delta()
{
	return cursor.delta;
}

const Position& scroll()
{
	return cursor.scroll;
}

struct InputListener :
	EventListener<KeyboardKeyDownEvent>,
	EventListener<KeyboardKeyUpEvent>,
	EventListener<MouseButtonDownEvent>,
	EventListener<MouseButtonUpEvent>,
	EventListener<MouseMoveEvent>,
	EventListener<MouseScrollEvent>,
	EventListener<MouseEnterEvent>,
	EventListener<MouseLeaveEvent>,
	EventListener<JoystickConnectedEvent>,
	EventListener<JoystickDisconnectedEvent>
{
	void onReceive(const KeyboardKeyDownEvent& event)
	{
		keyboard.pressedCount++;
		keyboard.down[static_cast<int>(event.key)] = true;
		keyboard.pressed[static_cast<int>(event.key)] = true;
		keyboard.timestamp[static_cast<int>(event.key)] = Time::unixtime().milliseconds();
	}
	void onReceive(const KeyboardKeyUpEvent& event)
	{
		keyboard.pressedCount--;
		keyboard.up[static_cast<int>(event.key)] = true;
		keyboard.pressed[static_cast<int>(event.key)] = false;
		keyboard.timestamp[static_cast<int>(event.key)] = 0;
	}
	void onReceive(const MouseButtonDownEvent& event)
	{
		cursor.down[static_cast<int>(event.button)] = true;
		cursor.pressed[static_cast<int>(event.button)] = true;
		cursor.timestamp[static_cast<int>(event.button)] = Time::unixtime().milliseconds();
	}
	void onReceive(const MouseButtonUpEvent& event)
	{
		cursor.up[static_cast<int>(event.button)] = true;
		cursor.pressed[static_cast<int>(event.button)] = false;
		cursor.timestamp[static_cast<int>(event.button)] = 0;
	}
	void onReceive(const MouseMoveEvent& event)
	{
		cursor.delta.x = event.x - cursor.position.x;
		cursor.delta.y = event.y - cursor.position.y;
		cursor.position.x = event.x;
		cursor.position.y = event.y;
	}
	void onReceive(const MouseScrollEvent& event)
	{
		cursor.scroll.x = event.x;
		cursor.scroll.y = event.y;
	}
	void onReceive(const MouseEnterEvent& event)
	{
		cursor.focus = true;
	}
	void onReceive(const MouseLeaveEvent& event)
	{
		cursor.focus = false;
	}
	void onReceive(const JoystickConnectedEvent& event)
	{
		Logger::info("Joystick connected !");
	}
	void onReceive(const JoystickDisconnectedEvent& event)
	{
		Logger::info("Joystick disconnected !");
	}
};

static InputListener* listener = nullptr;

}; // namespace input


void InputBackend::initialize()
{
	if (input::listener == nullptr)
		input::listener = new input::InputListener;
}

void InputBackend::destroy()
{
	delete input::listener;
	input::listener = nullptr;
}

void InputBackend::update()
{
	// Reset values that will not be reset by events
	for (uint32_t iKey = 0; iKey < input::getKeyCount(); iKey++)
		input::keyboard.down[iKey] = false;
	for (uint32_t iKey = 0; iKey < input::getKeyCount(); iKey++)
		input::keyboard.up[iKey] = false;
	for (uint32_t iButton = 0; iButton < input::getButtonCount(); iButton++)
		input::cursor.down[iButton] = false;
	for (uint32_t iButton = 0; iButton < input::getButtonCount(); iButton++)
		input::cursor.up[iButton] = false;
	input::cursor.delta = { 0.f };
	input::cursor.scroll = { 0.f };
	// Dispatch all input events
	EventDispatcher<input::KeyboardKeyDownEvent>::dispatch();
	EventDispatcher<input::KeyboardKeyUpEvent>::dispatch();
	EventDispatcher<input::MouseButtonDownEvent>::dispatch();
	EventDispatcher<input::MouseButtonUpEvent>::dispatch();
	EventDispatcher<input::MouseMoveEvent>::dispatch();
	EventDispatcher<input::MouseScrollEvent>::dispatch();
	EventDispatcher<input::MouseEnterEvent>::dispatch();
	EventDispatcher<input::MouseLeaveEvent>::dispatch();
	EventDispatcher<input::JoystickConnectedEvent>::dispatch();
	EventDispatcher<input::JoystickDisconnectedEvent>::dispatch();
}

};
