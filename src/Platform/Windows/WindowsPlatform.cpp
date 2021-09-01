#include <Aka/Platform/PlatformBackend.h>
#include <Aka/Platform/InputBackend.h>
#include <Aka/Platform/Platform.h>
#include <Aka/OS/Logger.h>

#include "WindowsPlatform.h"

#if defined(AKA_PLATFORM_WINDOWS)

#pragma comment(lib, "shlwapi.lib")

namespace aka {

const WORD terminalColors[20] = {
	0, // ForgeroundBlack
	FOREGROUND_RED, // ForegroundRed
	FOREGROUND_GREEN, // ForegroundGreen
	FOREGROUND_RED | FOREGROUND_GREEN, // ForegroundYellow
	FOREGROUND_BLUE, // ForegroundBlue
	FOREGROUND_RED | FOREGROUND_BLUE, // ForegroundMagenta
	FOREGROUND_GREEN | FOREGROUND_BLUE, // ForegroundCyan
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, // ForegroundWhite
	FOREGROUND_INTENSITY, // ForgeroundBrightBlack
	FOREGROUND_RED | FOREGROUND_INTENSITY, // ForegroundBrightRed
	FOREGROUND_GREEN | FOREGROUND_INTENSITY, // ForegroundBrightGreen
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, // ForegroundBrightYellow
	FOREGROUND_BLUE | FOREGROUND_INTENSITY, // ForegroundBrightBlue
	FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY, // ForegroundBrightMagenta
	FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, // ForegroundBrightCyan
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, // ForegroundBrightWhite
};

std::ostream& operator<<(std::ostream& os, Logger::Color color)
{
	if (color == Logger::Color::ForegroundNone)
		return os;
	HANDLE hdl = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleTextAttribute(hdl, terminalColors[(unsigned int)color]);
	return os;
}

void PlatformBackend::errorDialog(const std::string& message)
{
	StringWide msg = Utf8ToWchar(message);
	int msgBoxID = MessageBox(
		getWindowsWindowHandle(),
		msg.cstr(),
		L"Error",
		MB_ICONERROR | MB_OK | MB_DEFBUTTON1
	);
	switch (msgBoxID)
	{
	case IDOK:
		// TODO: add code
		break;
	}
}


}

#endif