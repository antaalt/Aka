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

struct Timestamp
{
	Timestamp();
private:
	Timestamp(uint64_t seconds);
	Timestamp& operator=(uint64_t seconds);
public:
	static Timestamp seconds(uint64_t seconds);
	// Timestamp initialized at zero
	static Timestamp zero();
	// Timestamp respresenting current unix time.
	static Timestamp now();

	uint64_t seconds() const;

	Timestamp operator+(const Timestamp& rhs) const;
	Timestamp operator-(const Timestamp& rhs) const;
	Timestamp& operator+=(const Timestamp& rhs);
	Timestamp& operator-=(const Timestamp& rhs);

	Timestamp operator%(const Timestamp& rhs) const;

	bool operator==(const Timestamp& rhs) const;
	bool operator!=(const Timestamp& rhs) const;
	bool operator<(const Timestamp& rhs) const;
	bool operator>(const Timestamp& rhs) const;
	bool operator<=(const Timestamp& rhs) const;
	bool operator>=(const Timestamp& rhs) const;
private:
	uint64_t m_timestamp;
};

struct Time
{
	Time();
private:
	Time(uint64_t milliseconds);
	Time& operator=(uint64_t milliseconds);
public:
	// Time initialized at zero
	static Time zero();
	// Time elapsed since app startup in ms
	static Time now();

	static Time milliseconds(uint64_t seconds);
	static Time seconds(float seconds);
	static Time from(Timestamp timestamp);

	uint64_t milliseconds() const;
	float seconds() const;
	Timestamp timestamp() const;

	Time operator+(const Time& rhs) const;
	Time operator-(const Time& rhs) const;
	Time& operator+=(const Time& rhs);
	Time& operator-=(const Time& rhs);

	Time operator%(const Time& rhs) const;

	bool operator==(const Time& rhs) const;
	bool operator!=(const Time& rhs) const;
	bool operator<(const Time& rhs) const;
	bool operator>(const Time& rhs) const;
	bool operator<=(const Time& rhs) const;
	bool operator>=(const Time& rhs) const;
private:
	// TODO use float for better resolution ?
	uint64_t m_value; // Internal resolution is milliseconds
};

};
