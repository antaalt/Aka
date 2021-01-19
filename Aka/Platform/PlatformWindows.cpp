#include "Platform.h"

namespace aka {

// https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
// http://cherrytree.at/misc/vk.htm
const unsigned int virtualKeyCodesCount = 256;
input::Key virtualKeyCodes[virtualKeyCodesCount];

void initKeyboard()
{
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

input::Key getKeyFromScancode(unsigned int scancode)
{
	// TODO handle numeric pad correctly & extended flag
	unsigned short code = scancode & 0x000000ff;
	// Extended key flag (set with ALT, CTRL, INS, DEL, HOME, END, PAGE UP, PAGE DOWN, Arrow keys, NUM LOCK, BREAK, PRINT SCRN, DIVIDE (NUMPAD), ENTER (NUMPAD)
	unsigned short enhanced = (scancode >> 8) & 0x000000ff;
	UINT virtualKey = MapVirtualKey(code, MAPVK_VSC_TO_VK_EX);
	//Logger::info("VK : ", virtualKey, " - code : ", code, "/", enhanced, "- name : ", getKeyName(virtualKeyCodes[virtualKey]));
	if (virtualKey >= virtualKeyCodesCount)
		return input::Key::Unknown;
	return virtualKeyCodes[virtualKey];
}

input::KeyboardLayout getKeyboardLayout()
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
