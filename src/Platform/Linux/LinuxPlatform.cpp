#include <Aka/Platform/PlatformBackend.h>
#include <Aka/Platform/InputBackend.h>
#include <Aka/Platform/Platform.h>
#include <Aka/OS/Logger.h>

#if defined(AKA_PLATFORM_LINUX)

namespace aka {

const unsigned int terminalColors[20] = {
	30, // ForgeroundBlack
	31, // ForegroundRed
	32, // ForegroundGreen
	33, // ForegroundYellow
	34, // ForegroundBlue
	35, // ForegroundMagenta
	36, // ForegroundCyan
	37, // ForegroundWhite
	90, // ForgeroundBrightBlack
	91, // ForegroundBrightRed
	92, // ForegroundBrightGreen
	93, // ForegroundBrightYellow
	94, // ForegroundBrightBlue 
	95, // ForegroundBrightMagenta
	96, // ForegroundBrightCyan
	97, // ForegroundBrightWhite
};

std::ostream& operator<<(std::ostream& os, Logger::Color color)
{
	if (color == Logger::Color::ForegroundNone)
		return os;
	return os << "\033[" << terminalColors[(unsigned int)color] << "m";
}


void PlatformBackend::errorDialog(const std::string& message)
{
}

};

#endif