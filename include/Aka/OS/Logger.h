#pragma once

#include <Aka/Platform/Platform.h>
#include <Aka/OS/Time.h>

#include <iostream>
#include <mutex>

namespace aka {

struct Logger {

	enum class Color {
		ForgeroundBlack,
		ForegroundRed,
		ForegroundGreen,
		ForegroundYellow,
		ForegroundBlue,
		ForegroundMagenta,
		ForegroundCyan,
		ForegroundWhite,
		ForgeroundBrightBlack,
		ForegroundBrightRed,
		ForegroundBrightGreen,
		ForegroundBrightYellow,
		ForegroundBrightBlue,
		ForegroundBrightMagenta,
		ForegroundBrightCyan,
		ForegroundBrightWhite,
		ForegroundNone,
		ForegroundDefault = ForegroundWhite,
	};

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

std::ostream& operator<<(std::ostream& os, Logger::Color color);

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
	ostream << Logger::Color::ForegroundWhite << std::endl; // Default color
}

template<typename ...Args>
inline void Logger::Channel::operator()(Args ...args)
{
	print(args...);
}

}
