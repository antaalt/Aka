#pragma once

#include "../Platform/Platform.h"
#include "Time.h"

#include <iostream>
#include <mutex>

namespace aka {

struct Logger {

	enum class Color {
#if defined(_WIN32)
		// FG
		FG_BLACK = 0,
		FG_RED = FOREGROUND_RED,
		FG_GREEN = FOREGROUND_GREEN,
		FG_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
		FG_BLUE = FOREGROUND_BLUE,
		FG_MAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
		FG_CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE,
		FG_WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		// FG_BRIGHT
		FG_BRIGHT_BLACK = FOREGROUND_INTENSITY,
		FG_BRIGHT_RED = FOREGROUND_RED | FOREGROUND_INTENSITY,
		FG_BRIGHT_GREEN = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
		FG_BRIGHT_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
		FG_BRIGHT_BLUE = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		FG_BRIGHT_MAGENTA = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		FG_BRIGHT_CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		FG_BRIGHT_WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		FG_DEFAULT = FG_WHITE,
		FG_NONE = 0
#else
		// FG
		FG_BLACK = 30,
		FG_RED,
		FG_GREEN,
		FG_YELLOW,
		FG_BLUE,
		FG_MAGENTA,
		FG_CYAN,
		FG_WHITE,
		// FG_BRIGHT
		FG_BRIGHT_BLACK = 90,
		FG_BRIGHT_RED,
		FG_BRIGHT_GREEN,
		FG_BRIGHT_YELLOW,
		FG_BRIGHT_BLUE,
		FG_BRIGHT_MAGENTA,
		FG_BRIGHT_CYAN,
		FG_BRIGHT_WHITE,
		FG_DEFAULT = FG_WHITE,
		FG_NONE = 0
#endif
	};
	static const Color defaultColor = Color::FG_WHITE;

	struct Channel {

		Channel(std::ostream& os, const std::string& name, Color color);

		template<typename ...Args>
		void print(Args ...args);

		template<typename ...Args>
		void operator()(Args ...args);

		void mute();

		void unmute();

	private:
		template<typename T, typename ...Args>
		void doPrint(std::ostream& out, T t, Args ...args) {
			out << t;
			doPrint(out, args...);
		}
		template<typename T>
		void doPrint(std::ostream& out, T t) {
			out << t;
		}
	private:
		static std::mutex writeLock;
		std::ostream& ostream;
		const std::string name;
		const Logger::Color color;
		bool muted;
	};

	static Channel critical;
	static Channel error;
	static Channel warn;
	static Channel info;
	static Channel debug;
};

inline std::ostream& operator<<(std::ostream& os, Logger::Color color)
{
	if (color == Logger::Color::FG_NONE)
		return os;
#if defined(_WIN32)
	HANDLE hdl = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hdl, static_cast<WORD>(color));
	return os;
#else
	return os << "\033[" << static_cast<int>(color) << "m";
#endif
}

template<typename ...Args>
inline void Logger::Channel::print(Args ...args)
{
	if (muted) return;
	Date date = Date::localtime();
	char buffer[26];
	int result = snprintf(buffer, 26, "[%04d-%02d-%02d %02d:%02d:%02d]",
		date.year,
		date.month,
		date.day,
		date.hour,
		date.minute,
		date.second
	);
	std::string head;
	head += (result == 0 ? "[Unknown time]" : buffer);
	head += "[";
	head += this->name;
	head += "]";
	std::lock_guard<std::mutex> m(this->writeLock);
	doPrint(this->ostream, this->color, head, args...);
	ostream << Logger::defaultColor << std::endl;
}

template<typename ...Args>
inline void Logger::Channel::operator()(Args ...args)
{
	print(args...);
}

}
