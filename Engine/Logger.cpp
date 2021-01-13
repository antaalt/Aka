#include "Logger.h"

namespace app {

Logger::Channel Logger::critical(std::cerr, "critic", Logger::Color::FG_BRIGHT_RED);
Logger::Channel Logger::error(std::cerr, "error", Logger::Color::FG_BRIGHT_MAGENTA);
Logger::Channel Logger::warn(std::cerr, "warn", Logger::Color::FG_BRIGHT_YELLOW);
Logger::Channel Logger::info(std::cout, "info", Logger::Color::FG_WHITE);
Logger::Channel Logger::debug(std::cout, "debug", Logger::Color::FG_BRIGHT_BLACK);

std::mutex Logger::Channel::writeLock;

Logger::Channel::Channel(std::ostream& os, const std::string& name, Logger::Color color) :
	ostream(os),
	name(name),
	color(color)
{
}

}