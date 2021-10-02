#include "..\..\include\Aka\OS\Time.h"
#include "..\..\include\Aka\OS\Time.h"
#include "..\..\include\Aka\OS\Time.h"
#include "..\..\include\Aka\OS\Time.h"
#include "..\..\include\Aka\OS\Time.h"
#include "..\..\include\Aka\OS\Time.h"
#include "..\..\include\Aka\OS\Time.h"
#include "..\..\include\Aka\OS\Time.h"
#include "..\..\include\Aka\OS\Time.h"
#include "..\..\include\Aka\OS\Time.h"
#include "..\..\include\Aka\OS\Time.h"
#include <Aka/OS/Time.h>

#include <chrono>

namespace aka {

using namespace std::chrono;

const time_point<steady_clock> g_startup = steady_clock::now();

Timestamp::Timestamp() :
	m_timestamp(0)
{
}

Timestamp::Timestamp(uint64_t seconds) :
	m_timestamp(seconds)
{
}

Timestamp& Timestamp::operator=(uint64_t seconds)
{
	m_timestamp = seconds;
	return *this;
}

Timestamp Timestamp::seconds(uint64_t seconds)
{
	return Timestamp(seconds);
}

Timestamp Timestamp::zero()
{
	return Timestamp();
}

Timestamp Timestamp::now()
{
	using namespace std::chrono;
	time_point<system_clock> now = system_clock::now();
	const duration<double> tse = now.time_since_epoch();
	seconds::rep secs = duration_cast<std::chrono::seconds>(tse).count();
	return Timestamp(secs);
}

uint64_t Timestamp::seconds() const
{
	return m_timestamp;
}

Timestamp Timestamp::operator+(const Timestamp& rhs) const
{
	return Timestamp(m_timestamp + rhs.m_timestamp);
}

Timestamp Timestamp::operator-(const Timestamp& rhs) const
{
	return Timestamp(m_timestamp - rhs.m_timestamp);
}

Timestamp& Timestamp::operator+=(const Timestamp& rhs)
{
	m_timestamp += rhs.m_timestamp;
	return *this;
}

Timestamp& Timestamp::operator-=(const Timestamp& rhs)
{
	m_timestamp -= rhs.m_timestamp;
	return *this;
}

Timestamp Timestamp::operator%(const Timestamp& rhs) const
{
	return Timestamp(m_timestamp % rhs.m_timestamp);
}

bool Timestamp::operator==(const Timestamp& rhs) const
{
	return m_timestamp == rhs.m_timestamp;
}

bool Timestamp::operator!=(const Timestamp& rhs) const
{
	return m_timestamp != rhs.m_timestamp;
}

bool Timestamp::operator<(const Timestamp& rhs) const
{
	return m_timestamp < rhs.m_timestamp;
}

bool Timestamp::operator>(const Timestamp& rhs) const
{
	return m_timestamp > rhs.m_timestamp;
}

bool Timestamp::operator<=(const Timestamp& rhs) const
{
	return m_timestamp <= rhs.m_timestamp;
}

bool Timestamp::operator>=(const Timestamp& rhs) const
{
	return m_timestamp >= rhs.m_timestamp;
}

Time Time::zero()
{
	return Time(0);
}

Time Time::now()
{
	using namespace std::chrono;
	return Time(static_cast<uint64_t>(duration_cast<std::chrono::milliseconds>(steady_clock::now() - g_startup).count()));
}

Time::Time() :
	m_value(0)
{
}

Time::Time(uint64_t milliseconds) :
	m_value(milliseconds)
{
}

Time& Time::operator=(uint64_t milliseconds)
{
	m_value = milliseconds;
	return *this;
}

Time Time::milliseconds(uint64_t milliseconds)
{
	return Time(milliseconds);
}

Time Time::seconds(float seconds)
{
	return Time(static_cast<uint64_t>(seconds * 1000.f));
}

Time Time::from(Timestamp timestamp)
{
	return Time(timestamp.seconds() * 1000);
}

uint64_t Time::milliseconds() const
{
	return m_value;
}

float Time::seconds() const
{
	return m_value / 1000.f;
}

Timestamp Time::timestamp() const
{
	return Timestamp::seconds(m_value / 1000);
}

Time Time::operator+(const Time& rhs) const
{
	return Time(m_value + rhs.m_value);
}

Time Time::operator-(const Time& rhs) const
{
	return Time(m_value - rhs.m_value);
}

Time& Time::operator+=(const Time& rhs)
{
	m_value += rhs.m_value;
	return *this;
}

Time& Time::operator-=(const Time& rhs)
{
	m_value -= rhs.m_value;
	return *this;
}

Time Time::operator%(const Time& rhs) const
{
	return Time(m_value % rhs.m_value);
}

bool Time::operator==(const Time& rhs) const
{
	return m_value == rhs.m_value;
}

bool Time::operator!=(const Time& rhs) const
{
	return m_value != rhs.m_value;
}

bool Time::operator<(const Time& rhs) const
{
	return m_value < rhs.m_value;
}

bool Time::operator>(const Time& rhs) const
{
	return m_value > rhs.m_value;
}

bool Time::operator<=(const Time& rhs) const
{
	return m_value <= rhs.m_value;
}

bool Time::operator>=(const Time& rhs) const
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
#if defined(AKA_PLATFORM_WINDOWS)
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
#if defined(AKA_PLATFORM_WINDOWS)
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