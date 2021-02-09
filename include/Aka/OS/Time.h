#pragma once

#include <stdint.h>
#include <Aka/Platform/Platform.h>

namespace aka {

struct Date
{
	// Get the date corresponding to the local timezone
	static Date localtime();
	// Get the date corresponding to the UTC time
	static Date globaltime();

	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
};

struct Time
{
	struct Unit {
		Unit();
	private:
		friend struct Time;
		Unit(uint64_t milliseconds);
		Unit& operator=(uint64_t milliseconds);
	public:
		static Time::Unit milliseconds(uint64_t seconds);
		static Time::Unit seconds(float seconds);

		uint64_t milliseconds() const;
		float seconds() const;

		Unit operator+(const Unit& rhs) const;
		Unit operator-(const Unit& rhs) const;
		Unit& operator+=(const Unit& rhs);
		Unit& operator-=(const Unit& rhs);

		Time::Unit operator%(const Unit& rhs) const;

		bool operator==(const Unit& rhs) const;
		bool operator!=(const Unit& rhs) const;
		bool operator<(const Unit& rhs) const;
		bool operator>(const Unit& rhs) const;
		bool operator<=(const Unit& rhs) const;
		bool operator>=(const Unit& rhs) const;
	private:
		uint64_t m_value;
	};

	// Time elapsed since app startup in ms
	static Time::Unit now();
	// Get unix timestamp
	static Time::Unit unixtime();
};

};
