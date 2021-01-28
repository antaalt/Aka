#include "Input.h"

#include "InputBackend.h"
#include "Platform.h"

namespace aka {
namespace input {

Keyboard keyboard;
Cursor cursor;

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
	case input::Key::NumLock: return "NumLock";
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
	case input::Key::Divide: return "Divide";
	case input::Key::Decimal: return "Decimal";
	case input::Key::Add: return "Add";
	case input::Key::Substract: return "Substract";
	case input::Key::Separator: return "Separator";
	case input::Key::Multiply: return "Multiply";
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
	case input::Key::Clear: return "Clear";
	case input::Key::End: return "End";
	case input::Key::Home: return "Home";
	case input::Key::F1: return "F1";
	case input::Key::F2: return "F2";
	case input::Key::F3: return "F3";
	case input::Key::F4: return "F4";
	case input::Key::F5: return "F5";
	case input::Key::F6: return "F6";
	case input::Key::F7: return "F7";
	case input::Key::F8: return "F8";
	case input::Key::F9: return "F9";
	case input::Key::F10: return "F10";
	case input::Key::F11: return "F11";
	case input::Key::F12: return "F12";
	}
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

}; // namespace input


// https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
// http://cherrytree.at/misc/vk.htm
const unsigned int virtualKeyCodesCount = 256;
input::Key virtualKeyCodes[virtualKeyCodesCount];

void InputBackend::initialize()
{
	for (uint32_t iKey = 0; iKey < input::g_keyboardKeyCount; iKey++)
	{
		input::keyboard.pressed[iKey] = false;
		input::keyboard.timestamp[iKey] = 0;
	}
	for (uint32_t iKey = 0; iKey < input::g_mouseButtonCount; iKey++)
	{
		input::cursor.pressed[iKey] = false;
		input::cursor.timestamp[iKey] = 0;
	}
	input::cursor.position = { 0.f };
	input::cursor.delta = { 0.f };
	input::cursor.scroll = { 0.f };

	memset(virtualKeyCodes, static_cast<int>(input::Key::Unknown), sizeof(input::Key) * virtualKeyCodesCount);
	virtualKeyCodes['A'] = input::Key::A;
	virtualKeyCodes['B'] = input::Key::B;
	virtualKeyCodes['C'] = input::Key::C;
	virtualKeyCodes['D'] = input::Key::D;
	virtualKeyCodes['E'] = input::Key::E;
	virtualKeyCodes['F'] = input::Key::F;
	virtualKeyCodes['G'] = input::Key::G;
	virtualKeyCodes['H'] = input::Key::H;
	virtualKeyCodes['I'] = input::Key::I;
	virtualKeyCodes['J'] = input::Key::J;
	virtualKeyCodes['K'] = input::Key::K;
	virtualKeyCodes['M'] = input::Key::M;
	virtualKeyCodes['M'] = input::Key::M;
	virtualKeyCodes['N'] = input::Key::N;
	virtualKeyCodes['O'] = input::Key::O;
	virtualKeyCodes['P'] = input::Key::P;
	virtualKeyCodes['Q'] = input::Key::Q;
	virtualKeyCodes['R'] = input::Key::R;
	virtualKeyCodes['S'] = input::Key::S;
	virtualKeyCodes['T'] = input::Key::T;
	virtualKeyCodes['U'] = input::Key::U;
	virtualKeyCodes['V'] = input::Key::V;
	virtualKeyCodes['W'] = input::Key::W;
	virtualKeyCodes['X'] = input::Key::X;
	virtualKeyCodes['Y'] = input::Key::Y;
	virtualKeyCodes['Z'] = input::Key::Z;

	virtualKeyCodes['0'] = input::Key::Num0;
	virtualKeyCodes['1'] = input::Key::Num1;
	virtualKeyCodes['2'] = input::Key::Num2;
	virtualKeyCodes['3'] = input::Key::Num3;
	virtualKeyCodes['4'] = input::Key::Num4;
	virtualKeyCodes['5'] = input::Key::Num5;
	virtualKeyCodes['6'] = input::Key::Num6;
	virtualKeyCodes['7'] = input::Key::Num7;
	virtualKeyCodes['8'] = input::Key::Num8;
	virtualKeyCodes['9'] = input::Key::Num9;

	virtualKeyCodes[VK_NUMLOCK] = input::Key::NumLock;
	virtualKeyCodes[VK_NUMPAD0] = input::Key::NumPad0;
	virtualKeyCodes[VK_NUMPAD1] = input::Key::NumPad1;
	virtualKeyCodes[VK_NUMPAD2] = input::Key::NumPad2;
	virtualKeyCodes[VK_NUMPAD3] = input::Key::NumPad3;
	virtualKeyCodes[VK_NUMPAD4] = input::Key::NumPad4;
	virtualKeyCodes[VK_NUMPAD5] = input::Key::NumPad5;
	virtualKeyCodes[VK_NUMPAD6] = input::Key::NumPad6;
	virtualKeyCodes[VK_NUMPAD7] = input::Key::NumPad7;
	virtualKeyCodes[VK_NUMPAD8] = input::Key::NumPad8;
	virtualKeyCodes[VK_NUMPAD9] = input::Key::NumPad9;
	virtualKeyCodes[VK_DIVIDE] = input::Key::Divide;
	virtualKeyCodes[VK_DECIMAL] = input::Key::Decimal;
	virtualKeyCodes[VK_ADD] = input::Key::Add;
	virtualKeyCodes[VK_SUBTRACT] = input::Key::Substract;
	virtualKeyCodes[VK_SEPARATOR] = input::Key::Separator;
	virtualKeyCodes[VK_MULTIPLY] = input::Key::Multiply;

	virtualKeyCodes[VK_LMENU] = input::Key::LeftAlt;
	virtualKeyCodes[VK_RMENU] = input::Key::RightAlt;
	virtualKeyCodes[VK_LSHIFT] = input::Key::LeftShift;
	virtualKeyCodes[VK_RSHIFT] = input::Key::RightShift;
	virtualKeyCodes[VK_LCONTROL] = input::Key::LeftCtrl;
	virtualKeyCodes[VK_RCONTROL] = input::Key::RightCtrl;
	virtualKeyCodes[VK_LSHIFT] = input::Key::LeftShift;
	virtualKeyCodes[VK_ESCAPE] = input::Key::Escape;
	virtualKeyCodes[VK_RETURN] = input::Key::Enter;
	virtualKeyCodes[VK_SPACE] = input::Key::Space;
	virtualKeyCodes[VK_BACK] = input::Key::BackSpace;
	virtualKeyCodes[VK_TAB] = input::Key::Tab;
	virtualKeyCodes[VK_LEFT] = input::Key::ArrowLeft;
	virtualKeyCodes[VK_RIGHT] = input::Key::ArrowRight;
	virtualKeyCodes[VK_UP] = input::Key::ArrowUp;
	virtualKeyCodes[VK_DOWN] = input::Key::ArrowDown;
	virtualKeyCodes[VK_CAPITAL] = input::Key::CapsLock;
	virtualKeyCodes[VK_PRIOR] = input::Key::PageUp;
	virtualKeyCodes[VK_NEXT] = input::Key::PageDown;
	virtualKeyCodes[VK_SNAPSHOT] = input::Key::PrintScreen;
	virtualKeyCodes[VK_END] = input::Key::End;
	virtualKeyCodes[VK_CLEAR] = input::Key::Clear;
	virtualKeyCodes[VK_HOME] = input::Key::Home;

	virtualKeyCodes[VK_F1] = input::Key::F1;
	virtualKeyCodes[VK_F2] = input::Key::F2;
	virtualKeyCodes[VK_F3] = input::Key::F3;
	virtualKeyCodes[VK_F4] = input::Key::F4;
	virtualKeyCodes[VK_F5] = input::Key::F5;
	virtualKeyCodes[VK_F6] = input::Key::F6;
	virtualKeyCodes[VK_F7] = input::Key::F7;
	virtualKeyCodes[VK_F8] = input::Key::F8;
	virtualKeyCodes[VK_F9] = input::Key::F9;
	virtualKeyCodes[VK_F10] = input::Key::F10;
	virtualKeyCodes[VK_F11] = input::Key::F11;
	virtualKeyCodes[VK_F12] = input::Key::F12;
}

void InputBackend::destroy()
{

}

void InputBackend::frame()
{
	for (uint32_t iKey = 0; iKey < input::g_keyboardKeyCount; iKey++)
		input::keyboard.down[iKey] = false;
	for (uint32_t iKey = 0; iKey < input::g_keyboardKeyCount; iKey++)
		input::keyboard.up[iKey] = false;
	for (uint32_t iKey = 0; iKey < input::g_mouseButtonCount; iKey++)
		input::cursor.down[iKey] = false;
	for (uint32_t iKey = 0; iKey < input::g_mouseButtonCount; iKey++)
		input::cursor.up[iKey] = false;
	input::cursor.delta = { 0.f };
	input::cursor.scroll = { 0.f };
}

void InputBackend::onKeyDown(input::Key key)
{
	input::keyboard.down[static_cast<int>(key)] = true;
	input::keyboard.pressed[static_cast<int>(key)] = true;
	input::keyboard.timestamp[static_cast<int>(key)] = 0;
}

void InputBackend::onKeyUp(input::Key key)
{
	input::keyboard.up[static_cast<int>(key)] = true;
	input::keyboard.pressed[static_cast<int>(key)] = false;
	input::keyboard.timestamp[static_cast<int>(key)] = 0;
}

void InputBackend::onMouseButtonDown(input::Button button)
{
	input::cursor.down[static_cast<int>(button)] = true;
	input::cursor.pressed[static_cast<int>(button)] = true;
	input::cursor.timestamp[static_cast<int>(button)] = 0;
}

void InputBackend::onMouseButtonUp(input::Button button)
{
	input::cursor.up[static_cast<int>(button)] = true;
	input::cursor.pressed[static_cast<int>(button)] = false;
	input::cursor.timestamp[static_cast<int>(button)] = 0;
}

void InputBackend::onMouseMove(float x, float y)
{
	input::cursor.delta.x = x - input::cursor.position.x;
	input::cursor.delta.y = y - input::cursor.position.y;
	input::cursor.position.x = x;
	input::cursor.position.y = y;
}

void InputBackend::onMouseScroll(float x, float y)
{
	input::cursor.scroll.x = x;
	input::cursor.scroll.y = y;
}

input::Key InputBackend::getKeyFromScancode(unsigned int scancode)
{
	// TODO handle numeric pad correctly & extended flag
	unsigned short code = scancode & 0x000000ff;
	// Extended key flag (set with ALT, CTRL, INS, DEL, HOME, END, PAGE UP, PAGE DOWN, Arrow keys, NUM LOCK, BREAK, PRINT SCRN, DIVIDE (NUMPAD), ENTER (NUMPAD)
	unsigned short enhanced = (scancode >> 8) & 0x000000ff;
	UINT virtualKey = MapVirtualKey(code, MAPVK_VSC_TO_VK_EX);
	if (virtualKey >= virtualKeyCodesCount)
		return input::Key::Unknown;
	return virtualKeyCodes[virtualKey];
}

input::KeyboardLayout InputBackend::getKeyboardLayout()
{
	/*HWND hwnd = GetForegroundWindow();
	if (hwnd) {
		DWORD threadID = GetWindowThreadProcessId(hwnd, NULL);
		HKL currentLayout = GetKeyboardLayout(threadID);
		unsigned int x = (unsigned int)currentLayout & 0x0000FFFF;

		WORD languageIdentifier = LOWORD(currentLayout);
		WORD deviceHdlToPhysicalLayout = HIWORD(currentLayout);
		//std::cout << languageIdentifier << " - " << deviceHdlToPhysicalLayout << " - " << MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH) << " - " << MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_UK) << std::endl;

		//ToAsciiEx(VirtualKey, ScanCode, KeyState, (LPWORD)&wd, 0, hKL);

		return input::KeyboardLayout::Azerty;
	}*/
	return input::KeyboardLayout::Default;
}

};
