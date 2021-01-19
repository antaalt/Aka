#include "Time.h"

#include <chrono>

namespace aka {

using namespace std::chrono;

const time_point<steady_clock> g_startup = steady_clock::now();

Time::Unit Time::now()
{
	using namespace std::chrono;
	return Unit(static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now() - g_startup).count()));
}

Time::Unit Time::unix()
{
	using namespace std::chrono;
	time_point<system_clock> now = system_clock::now();
	const duration<double> tse = now.time_since_epoch();
	seconds::rep milli = duration_cast<milliseconds>(tse).count() % 1000;
	return Time::Unit(milli);
}

Time::Unit::Unit() :
	m_value(0)
{
}

Time::Unit::Unit(uint64_t milliseconds) :
	m_value(milliseconds)
{
}

Time::Unit& Time::Unit::operator=(uint64_t milliseconds)
{
	m_value = milliseconds;
	return *this;
}

Time::Unit Time::Unit::milliseconds(uint64_t milliseconds)
{
	return Time::Unit(milliseconds);
}

Time::Unit Time::Unit::seconds(float seconds)
{
	return Time::Unit(static_cast<uint64_t>(seconds * 1000.f));
}

uint64_t Time::Unit::milliseconds() const
{
	return m_value;
}

float Time::Unit::seconds() const
{
	return m_value / 1000.f;
}

Time::Unit Time::Unit::operator+(const Unit& rhs) const
{
	return Unit(m_value + rhs.m_value);
}

Time::Unit Time::Unit::operator-(const Unit& rhs) const
{
	return Unit(m_value - rhs.m_value);
}

Time::Unit& Time::Unit::operator+=(const Unit& rhs)
{
	m_value += rhs.m_value;
	return *this;
}

Time::Unit& Time::Unit::operator-=(const Unit& rhs)
{
	m_value -= rhs.m_value;
	return *this;
}

Time::Unit Time::Unit::operator%(const Unit& rhs) const
{
	return Time::Unit(m_value % rhs.m_value);
}

bool Time::Unit::operator==(const Unit& rhs) const
{
	return m_value == rhs.m_value;
}

bool Time::Unit::operator!=(const Unit& rhs) const
{
	return m_value != rhs.m_value;
}

bool Time::Unit::operator<(const Unit& rhs) const
{
	return m_value < rhs.m_value;
}

bool Time::Unit::operator>(const Unit& rhs) const
{
	return m_value > rhs.m_value;
}

bool Time::Unit::operator<=(const Unit& rhs) const
{
	return m_value <= rhs.m_value;
}

bool Time::Unit::operator>=(const Unit& rhs) const
{
	return m_value >= rhs.m_value;
}

Date Date::localtime()
{
	using namespace std::chrono;
	time_point<system_clock> now = system_clock::now();
	time_t t = system_clock::to_time_t(now);
	std::tm lt{};
	// Safe localtime for multithread issues
#if defined(_WIN32)
	localtime_s(&lt, &t);
#else
	localtime_r(&t, &lt);
#endif
	Date date;
	date.year = lt.tm_year + 1900;
	date.month = lt.tm_mon + 1;
	date.day = lt.tm_mday;
	date.hour = lt.tm_mday;
	date.minute = lt.tm_min;
	date.second = lt.tm_sec;
	return date;
}

Date Date::globaltime()
{
	using namespace std::chrono;
	time_point<system_clock> now = system_clock::now();
	time_t t = system_clock::to_time_t(now);
	std::tm lt{};
	// Safe gmtime for multithread issues
#if defined(_WIN32)
	gmtime_s(&lt, &t);
#else
	gmtime_r(&t, &lt);
#endif
	Date date;
	date.year = lt.tm_year + 1900;
	date.month = lt.tm_mon + 1;
	date.day = lt.tm_mday;
	date.hour = lt.tm_mday;
	date.minute = lt.tm_min;
	date.second = lt.tm_sec;
	return date;
}

}