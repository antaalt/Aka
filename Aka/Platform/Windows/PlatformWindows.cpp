#include "../InputBackend.h"
#include "../PlatformBackend.h"
#include "../Platform.h"
#include "../../OS/FileSystem.h"

#if defined(AKA_WINDOWS)

#include <string>
#include <fstream>
#include <utf8.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")  

namespace aka {

// https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
// http://cherrytree.at/misc/vk.htm
const unsigned int virtualKeyCodesCount = 256;
input::Key virtualKeyCodes[]{
	input::Key::Unknown,
	input::Key::Unknown, // VK_LBUTTON
	input::Key::Unknown, // VK_RBUTTON
	input::Key::Unknown, // VK_CANCEL
	input::Key::Unknown, // VK_MBUTTON
	input::Key::Unknown, // VK_XBUTTON1
	input::Key::Unknown, // VK_XBUTTON2
	input::Key::Unknown,
	input::Key::BackSpace, // VK_BACK
	input::Key::Tab, // VK_TAB
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Clear, // VK_CLEAR
	input::Key::Enter, // VK_RETURN
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown, // VK_SHIFT
	input::Key::Unknown, // VK_CONTROL
	input::Key::Unknown, // VK_MENU
	input::Key::Unknown, // VK_PAUSE
	input::Key::CapsLock, // VK_CAPITAL
	input::Key::Unknown, // VK_KANA / VK_HANGUEL / VK_HANGUL
	input::Key::Unknown, // VK_IME_ON
	input::Key::Unknown, // VK_JUNJA
	input::Key::Unknown, // VK_FINAL
	input::Key::Unknown, // VK_HANJA / VK_KANJI
	input::Key::Unknown, // VK_IME_OFF
	input::Key::Escape, // VK_ESCAPE
	input::Key::Unknown, // VK_CONVERT
	input::Key::Unknown, // VK_NONCONVERT
	input::Key::Unknown, // VK_ACCEPT
	input::Key::Unknown, // VK_MODECHANGE
	input::Key::Space, // VK_SPACE
	input::Key::PageUp, // VK_PRIOR
	input::Key::PageDown, // VK_NEXT
	input::Key::End, // VK_END
	input::Key::Home, // VK_HOME
	input::Key::ArrowLeft, // VK_LEFT
	input::Key::ArrowUp, // VK_UP
	input::Key::ArrowRight, // VK_RIGHT
	input::Key::ArrowDown, // VK_DOWN
	input::Key::Unknown, // VK_SELECT
	input::Key::Unknown, // VK_PRINT
	input::Key::Unknown, // VK_EXECUTE
	input::Key::PrintScreen, // VK_SNAPSHOT
	input::Key::Unknown, // VK_INSERT
	input::Key::Unknown, // VK_DELETE
	input::Key::Unknown, // VK_HELP
	input::Key::Num0, // 0
	input::Key::Num1, // 1
	input::Key::Num2, // 2
	input::Key::Num3, // 3
	input::Key::Num4, // 4
	input::Key::Num5, // 5
	input::Key::Num6, // 6
	input::Key::Num7, // 7
	input::Key::Num8, // 8
	input::Key::Num9, // 9
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::A, // A
	input::Key::B, // B
	input::Key::C, // C
	input::Key::D, // D
	input::Key::E, // E
	input::Key::F, // F
	input::Key::G, // G
	input::Key::H, // H
	input::Key::I, // I
	input::Key::J, // J
	input::Key::K, // K
	input::Key::L, // L
	input::Key::M, // M
	input::Key::N, // N
	input::Key::O, // O
	input::Key::P, // P
	input::Key::Q, // Q
	input::Key::R, // R
	input::Key::S, // S
	input::Key::T, // T
	input::Key::U, // U
	input::Key::V, // V
	input::Key::W, // W
	input::Key::X, // X
	input::Key::Y, // Y
	input::Key::Z, // Z
	input::Key::Unknown, // VK_LWIN
	input::Key::Unknown, // VK_RWIN
	input::Key::Unknown, // VK_APPS
	input::Key::Unknown,
	input::Key::Unknown, // VK_SLEEP
	input::Key::NumPad0, // VK_NUMPAD0
	input::Key::NumPad1, // VK_NUMPAD1
	input::Key::NumPad2, // VK_NUMPAD2
	input::Key::NumPad3, // VK_NUMPAD3
	input::Key::NumPad4, // VK_NUMPAD4
	input::Key::NumPad5, // VK_NUMPAD5
	input::Key::NumPad6, // VK_NUMPAD6
	input::Key::NumPad7, // VK_NUMPAD7
	input::Key::NumPad8, // VK_NUMPAD8
	input::Key::NumPad9, // VK_NUMPAD9
	input::Key::Multiply, // VK_MULTIPLY
	input::Key::Add, // VK_ADD
	input::Key::Separator, // VK_SEPARATOR
	input::Key::Substract, // VK_SUBTRACT
	input::Key::Decimal, // VK_DECIMAL
	input::Key::Divide, // VK_DIVIDE
	input::Key::F1, // VK_F1
	input::Key::F2, // VK_F2
	input::Key::F3, // VK_F3
	input::Key::F4, // VK_F4
	input::Key::F5, // VK_F5
	input::Key::F6, // VK_F6
	input::Key::F7, // VK_F7
	input::Key::F8, // VK_F8
	input::Key::F9, // VK_F9
	input::Key::F10, // VK_F10
	input::Key::F11, // VK_F11
	input::Key::F12, // VK_F12
	input::Key::Unknown, // VK_F13
	input::Key::Unknown, // VK_F14
	input::Key::Unknown, // VK_F15
	input::Key::Unknown, // VK_F16
	input::Key::Unknown, // VK_F17
	input::Key::Unknown, // VK_F18
	input::Key::Unknown, // VK_F19
	input::Key::Unknown, // VK_F20
	input::Key::Unknown, // VK_F21
	input::Key::Unknown, // VK_F22
	input::Key::Unknown, // VK_F23
	input::Key::Unknown, // VK_F24
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::NumLock, // VK_NUMLOCK
	input::Key::Unknown, // VK_SCROLL
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::LeftShift, // VK_LSHIFT
	input::Key::RightShift, // VK_RSHIFT
	input::Key::LeftCtrl, // VK_LCONTROL
	input::Key::RightCtrl, // VK_RCONTROL
	input::Key::LeftAlt, // VK_LMENU
	input::Key::RightAlt, // VK_RMENU
	input::Key::Unknown, // VK_BROWSER_BACK
	input::Key::Unknown, // VK_BROWSER_FORWARD
	input::Key::Unknown, // VK_BROWSER_REFRESH
	input::Key::Unknown, // VK_BROWSER_STOP
	input::Key::Unknown, // VK_BROWSER_SEARCH
	input::Key::Unknown, // VK_BROWSER_FAVORITES
	input::Key::Unknown, // VK_BROWSER_HOME
	input::Key::Unknown, // VK_VOLUME_MUTE
	input::Key::Unknown, // VK_VOLUME_DOWN
	input::Key::Unknown, // VK_VOLUME_UP
	input::Key::Unknown, // VK_MEDIA_NEXT_TRACK
	input::Key::Unknown, // VK_MEDIA_PREV_TRACK
	input::Key::Unknown, // VK_MEDIA_STOP
	input::Key::Unknown, // VK_MEDIA_PLAY_PAUSE
	input::Key::Unknown, // VK_LAUNCH_MAIL
	input::Key::Unknown, // VK_LAUNCH_MEDIA_SELECT
	input::Key::Unknown, // VK_LAUNCH_APP1
	input::Key::Unknown, // VK_LAUNCH_APP2
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown, // VK_OEM_1
	input::Key::Unknown, // VK_OEM_PLUS
	input::Key::Unknown, // VK_OEM_COMMA
	input::Key::Unknown, // VK_OEM_MINUS
	input::Key::Unknown, // VK_OEM_PERIOD
	input::Key::Unknown, // VK_OEM_2
	input::Key::Unknown, // VK_OEM_3
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown,
	input::Key::Unknown
};


input::Key InputBackend::getKeyFromScancode(int scancode)
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


bool PlatformBackend::directoryExist(const Path& path)
{
	std::wstring str;
	utf8::utf8to16(path.str().begin(), path.str().end(), std::back_inserter(str));
	DWORD ftyp = GetFileAttributes(str.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false; // Incorrect path
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true; // Directory
	return false;
}
bool PlatformBackend::directoryCreate(const Path& path)
{
	size_t pos = 0;
	const std::string& str = path.str();
	do
	{
		pos = str.find_first_of("\\/", pos + 1);
		if (pos == str.length())
			return true;
		std::string p = str.substr(0, pos);
		if (p == "." || p == ".." || p == "/" || p == "\\")
			continue;
		std::wstring wstr;
		utf8::utf8to16(p.begin(), p.end(), std::back_inserter(wstr));
		if (!CreateDirectory(wstr.c_str(), NULL))
		{
			DWORD error = GetLastError();
			if (ERROR_ALREADY_EXISTS == error)
				continue;
			else if (ERROR_SUCCESS != error)
				return false;
		}
	} while (pos != std::string::npos);
	return true;
}
bool PlatformBackend::directoryRemove(const Path& path, bool recursive)
{
	if (recursive)
	{
		std::wstring wstr;
		utf8::utf8to16(path.str().begin(), path.str().end(), std::back_inserter(wstr));
		WIN32_FIND_DATA data;
		wchar_t cwd[512];
		GetCurrentDirectory(512, cwd);
		SetCurrentDirectory(wstr.c_str());
		HANDLE hFind = FindFirstFile(L"*", &data);
		do {
			if (data.dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY)
				DeleteFile(data.cFileName);
			else
			{
				// TODO delete folder recursively
			}
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
		SetCurrentDirectory(cwd);
	}
	std::wstring str;
	utf8::utf8to16(path.str().begin(), path.str().end(), std::back_inserter(str));
	return RemoveDirectory(str.c_str()) == TRUE;
}
bool PlatformBackend::fileExist(const Path& path)
{
	std::wstring str;
	utf8::utf8to16(path.str().begin(), path.str().end(), std::back_inserter(str));
	DWORD ftyp = GetFileAttributes(str.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!
	return (!(ftyp & FILE_ATTRIBUTE_DIRECTORY));
}
bool PlatformBackend::fileCreate(const Path& path)
{
	std::ofstream file(path.str());
	return file.is_open();
}
bool PlatformBackend::fileRemove(const Path& path)
{
	std::wstring str;
	utf8::utf8to16(path.str().begin(), path.str().end(), std::back_inserter(str));
	return DeleteFile(str.c_str()) == TRUE;
}

std::vector<Path> PlatformBackend::enumerate(const Path& path)
{
	const wchar_t separator = '/';
	std::wstring str;
	utf8::utf8to16(path.str().begin(), path.str().end(), std::back_inserter(str));
	WIN32_FIND_DATA data;
	std::wstring searchString;
	if (str.back() == separator)
		searchString = str + L'*';
	else
		searchString = str + separator + L'*';
	HANDLE hFind = FindFirstFile(searchString.c_str(), &data);
	std::vector<Path> paths;
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (data.dwFileAttributes == INVALID_FILE_ATTRIBUTES)
				continue;
			else if (wcscmp(data.cFileName, L"..") == 0)
				continue;
			else if (wcscmp(data.cFileName, L".") == 0)
				continue;
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// directory
				std::string str;
				utf8::utf16to8(data.cFileName, data.cFileName + wcslen(data.cFileName), std::back_inserter(str));
				paths.push_back(path.str() + str);
			}
			else
			{
				// file
				std::string str;
				utf8::utf16to8(data.cFileName, data.cFileName + wcslen(data.cFileName), std::back_inserter(str));
				paths.push_back(path.str() + str);
				// size :((unsigned long long)data.nFileSizeHigh * ((unsigned long long)MAXDWORD + 1ULL)) + (unsigned long long)data.nFileSizeLow;
			}
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
	return paths;
}

Path PlatformBackend::normalize(const Path& path)
{
	// On windows, correct path should be only with '\\'
	// Remove ../ & ./ aswell.
	return path;
}

Path PlatformBackend::executablePath()
{
	wchar_t path[MAX_PATH]{};
	if (GetModuleFileName(NULL, path, MAX_PATH) == 0)
		return Path();
	std::string str;
	utf8::utf16to8(path, path + wcslen(path), std::back_inserter(str));
	return Path(str);
}

Path PlatformBackend::cwd()
{
	WCHAR path[MAX_PATH] = { 0 };
	if (GetCurrentDirectory(MAX_PATH, path) == 0)
		return Path();
	std::string str;
	utf8::utf16to8(path, path + wcslen(path), std::back_inserter(str));
	return Path(str + '\\');
}

std::string PlatformBackend::extension(const Path& path)
{
	std::wstring str;
	utf8::utf8to16(path.str().begin(), path.str().end(), std::back_inserter(str));
	LPWSTR extension = PathFindExtension(str.c_str());
	std::string out;
	utf8::utf16to8(extension, extension + wcslen(extension), std::back_inserter(out));
	if (out.size() < 1)
		return std::string();
	return out.substr(1, out.size() - 1);
}

std::string PlatformBackend::fileName(const Path& path)
{
	std::wstring str;
	utf8::utf8to16(path.str().begin(), path.str().end(), std::back_inserter(str));
	LPWSTR extension = PathFindFileName(str.c_str());
	std::string out;
	utf8::utf16to8(extension, extension + wcslen(extension), std::back_inserter(out));
	return out;
}

};
#endif