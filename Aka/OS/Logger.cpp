#include "Logger.h"

namespace aka {

Logger::Channel Logger::critical(std::cerr, "critic", Logger::Color::ForegroundBrightRed);
Logger::Channel Logger::error(std::cerr, "error", Logger::Color::ForegroundBrightMagenta);
Logger::Channel Logger::warn(std::cerr, "warn", Logger::Color::ForegroundBrightYellow);
Logger::Channel Logger::info(std::cout, "info", Logger::Color::ForegroundWhite);
Logger::Channel Logger::debug(std::cout, "debug", Logger::Color::ForgeroundBrightBlack);

std::mutex Logger::Channel::writeLock;

Logger::Channel::Channel(std::ostream& os, const std::string& name, Logger::Color color) :
	ostream(os),
	name(name),
	color(color),
	muted(false)
{
}

void Logger::Channel::mute()
{
	muted = true;
}

void Logger::Channel::unmute()
{
	muted = false;
}

}