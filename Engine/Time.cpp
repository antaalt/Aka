#include "Time.h"

#include <chrono>

namespace app {

const std::chrono::time_point<std::chrono::steady_clock> g_startup = std::chrono::steady_clock::now();

Time::Unit Time::now()
{
	return Unit(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - g_startup).count()));
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

const uint64_t& Time::Unit::operator()() const
{
	return m_value;
}

uint64_t& Time::Unit::operator()()
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

}