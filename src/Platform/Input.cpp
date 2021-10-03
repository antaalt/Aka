#include <Aka/Platform/Input.h>

#include <Aka/OS/Time.h>
#include <Aka/Core/Event.h>

#include <iostream>
#include <map>

namespace aka {

std::ostream& operator<<(std::ostream& os, const KeyboardKey key)
{
	return (os << Keyboard::name(key));
}

std::ostream& operator<<(std::ostream& os, const KeyboardLayout layout)
{
	return (os << Keyboard::layoutName(layout));
}

std::ostream& operator<<(std::ostream& os, const MouseButton button)
{
	return (os << Mouse::name(button));
}

std::ostream& operator<<(std::ostream& os, const GamepadButton button)
{
	return (os << Gamepad::name(button));
}

std::ostream& operator<<(std::ostream& os, const GamepadAxis axis)
{
	return (os << Gamepad::name(axis));
}

const char* Keyboard::name(KeyboardKey key)
{
	switch (key)
	{
	default: return "Unknown";
	case KeyboardKey::A: return "A";
	case KeyboardKey::B: return "B";
	case KeyboardKey::C: return "C";
	case KeyboardKey::D: return "D";
	case KeyboardKey::E: return "E";
	case KeyboardKey::F: return "F";
	case KeyboardKey::G: return "G";
	case KeyboardKey::H: return "H";
	case KeyboardKey::I: return "I";
	case KeyboardKey::J: return "J";
	case KeyboardKey::K: return "K";
	case KeyboardKey::L: return "L";
	case KeyboardKey::M: return "M";
	case KeyboardKey::N: return "N";
	case KeyboardKey::O: return "O";
	case KeyboardKey::P: return "P";
	case KeyboardKey::Q: return "Q";
	case KeyboardKey::R: return "R";
	case KeyboardKey::S: return "S";
	case KeyboardKey::T: return "T";
	case KeyboardKey::U: return "U";
	case KeyboardKey::V: return "V";
	case KeyboardKey::W: return "W";
	case KeyboardKey::X: return "X";
	case KeyboardKey::Y: return "Y";
	case KeyboardKey::Z: return "Z";
	case KeyboardKey::Num0: return "Num0";
	case KeyboardKey::Num1: return "Num1";
	case KeyboardKey::Num2: return "Num2";
	case KeyboardKey::Num3: return "Num3";
	case KeyboardKey::Num4: return "Num4";
	case KeyboardKey::Num5: return "Num5";
	case KeyboardKey::Num6: return "Num6";
	case KeyboardKey::Num7: return "Num7";
	case KeyboardKey::Num8: return "Num8";
	case KeyboardKey::Num9: return "Num9";
	case KeyboardKey::NumLock: return "NumLock";
	case KeyboardKey::KeyPad0: return "KeyPad0";
	case KeyboardKey::KeyPad1: return "KeyPad1";
	case KeyboardKey::KeyPad2: return "KeyPad2";
	case KeyboardKey::KeyPad3: return "KeyPad3";
	case KeyboardKey::KeyPad4: return "KeyPad4";
	case KeyboardKey::KeyPad5: return "KeyPad5";
	case KeyboardKey::KeyPad6: return "KeyPad6";
	case KeyboardKey::KeyPad7: return "KeyPad7";
	case KeyboardKey::KeyPad8: return "KeyPad8";
	case KeyboardKey::KeyPad9: return "KeyPad9";
	case KeyboardKey::KeyPadDivide: return "KeyPadDivide";
	case KeyboardKey::KeyPadDecimal: return "KeyPadDecimal";
	case KeyboardKey::KeyPadAdd: return "KeyPadAdd";
	case KeyboardKey::KeyPadSubstract: return "KeyPadSubstract";
	case KeyboardKey::KeyPadMultiply: return "KeyPadMultiply";
	case KeyboardKey::KeyPadEnter: return "KeyPadEnter";
	case KeyboardKey::AltRight: return "AltRight";
	case KeyboardKey::AltLeft: return "AltLeft";
	case KeyboardKey::ControlRight: return "ControlRight";
	case KeyboardKey::ControlLeft: return "ControlLeft";
	case KeyboardKey::ShiftRight: return "ShiftRight";
	case KeyboardKey::ShiftLeft: return "ShiftLeft";
	case KeyboardKey::Escape: return "Escape";
	case KeyboardKey::Enter: return "Enter";
	case KeyboardKey::Space: return "Space";
	case KeyboardKey::BackSpace: return "BackSpace";
	case KeyboardKey::Tab: return "Tab";
	case KeyboardKey::ArrowLeft: return "ArrowLeft";
	case KeyboardKey::ArrowRight: return "ArrowRight";
	case KeyboardKey::ArrowUp: return "ArrowUp";
	case KeyboardKey::ArrowDown: return "ArrowDown";
	case KeyboardKey::CapsLock: return "CapsLock";
	case KeyboardKey::PageUp: return "PageUp";
	case KeyboardKey::PageDown: return "PageDown";
	case KeyboardKey::Pause: return "Pause";
	case KeyboardKey::Menu: return "Menu";
	case KeyboardKey::PrintScreen: return "PrintScreen";
	case KeyboardKey::Insert: return "Insert";
	case KeyboardKey::Delete: return "Delete";
	case KeyboardKey::Help: return "Help";
	case KeyboardKey::Clear: return "Clear";
	case KeyboardKey::ScrollLock: return "ScrollLock";
	case KeyboardKey::End: return "End";
	case KeyboardKey::Home: return "Home";
	case KeyboardKey::Slash: return "Slash";
	case KeyboardKey::BackSlash: return "BackSlash";
	case KeyboardKey::Plus: return "Plus";
	case KeyboardKey::Minus: return "Minus";
	case KeyboardKey::Equal: return "Equal";
	case KeyboardKey::Comma: return "Comma";
	case KeyboardKey::Period: return "Period";
	case KeyboardKey::Semicolon: return "Semicolon";
	case KeyboardKey::Apostrophe: return "Apostrophe";
	case KeyboardKey::Grave: return "Grave";
	case KeyboardKey::BracketAngle: return "BracketAngle";
	case KeyboardKey::BracketLeft: return "BracketLeft";
	case KeyboardKey::BracketRight: return "BracketRight";
	case KeyboardKey::AltPrintScreen: return "AltPrintScreen";
	case KeyboardKey::F1: return "F1";
	case KeyboardKey::F2: return "F2";
	case KeyboardKey::F3: return "F3";
	case KeyboardKey::F4: return "F4";
	case KeyboardKey::F5: return "F5";
	case KeyboardKey::F6: return "F6";
	case KeyboardKey::F7: return "F7";
	case KeyboardKey::F8: return "F8";
	case KeyboardKey::F9: return "F9";
	case KeyboardKey::F10: return "F10";
	case KeyboardKey::F11: return "F11";
	case KeyboardKey::F12: return "F12";
	}
}

const char* Mouse::name(MouseButton button)
{
	switch (button)
	{
	default: return "Unknown";
	case MouseButton::Button1: return "ButtonLeft";
	case MouseButton::Button2: return "ButtonRight";
	case MouseButton::Button3: return "ButtonMiddle";
	case MouseButton::Button4: return "Button4";
	case MouseButton::Button5: return "Button5";
	case MouseButton::Button6: return "Button6";
	case MouseButton::Button7: return "Button7";
	case MouseButton::Button8: return "Button8";
	}
}

const char* Keyboard::layoutName(KeyboardLayout layout)
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

const char* Gamepad::name(GamepadButton button)
{
	switch (button)
	{
	default: return "Unknown";
	case GamepadButton::A: return "A";
	case GamepadButton::B: return "B";
	case GamepadButton::X: return "X";
	case GamepadButton::Y: return "Y";
	case GamepadButton::LeftBumper: return "LeftBumper";
	case GamepadButton::RightBumper: return "RightBumper";
	case GamepadButton::Select: return "Select";
	case GamepadButton::Start: return "Start";
	case GamepadButton::Home: return "Home";
	case GamepadButton::LeftStick: return "LeftStick";
	case GamepadButton::RightStick: return "RightStick";
	case GamepadButton::DpadUp: return "DpadUp";
	case GamepadButton::DpadRight: return "DpadRight";
	case GamepadButton::DpadLeft: return "DpadLeft";
	case GamepadButton::DpadDown: return "DpadDown";
	}
}

const char* Gamepad::name(GamepadAxis axis)
{
	switch (axis)
	{
	default: return "Unknown";
	case GamepadAxis::Left: return "Left";
	case GamepadAxis::Right: return "Right";
	case GamepadAxis::TriggerLeft: return "TriggerLeft";
	case GamepadAxis::TriggerRight: return "TriggerRight";
	}
}

KeyboardLayout Keyboard::layout() const
{
	return m_layout;
}

bool Keyboard::down(KeyboardKey key) const
{
	return m_down[static_cast<int>(key)];
}

bool Keyboard::up(KeyboardKey key) const
{
	return m_up[static_cast<int>(key)];
}

bool Keyboard::pressed(KeyboardKey key) const
{
	return m_pressed[static_cast<int>(key)];
}

bool Keyboard::any() const
{
	return m_pressedCount > 0;
}

bool Mouse::down(MouseButton button) const
{
	return m_down[static_cast<int>(button)];
}

bool Mouse::up(MouseButton button) const
{
	return m_up[static_cast<int>(button)];
}

bool Mouse::pressed(MouseButton button) const
{
	return m_pressed[static_cast<int>(button)];
}

bool Mouse::focused() const
{
	return m_focus;
}

const Position& Mouse::position() const
{
	return m_position;
}

const Position& Mouse::delta() const
{
	return m_delta;
}

const Position& Mouse::scroll() const
{
	return m_scroll;
}

const char* Gamepad::name(GamepadID gid) const
{
	return m_name;
}

bool Gamepad::down(GamepadID gid, GamepadButton button) const
{
	return m_down[static_cast<int>(button)];
}

bool Gamepad::up(GamepadID gid, GamepadButton button) const
{
	return m_up[static_cast<int>(button)];
}

bool Gamepad::pressed(GamepadID gid, GamepadButton button) const
{
	return m_pressed[static_cast<int>(button)];
}

const Position& Gamepad::axis(GamepadID gid, GamepadAxis axis) const
{
	return m_axes[static_cast<int>(axis)];
}

};
