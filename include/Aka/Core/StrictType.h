#pragma once

#include <iostream>

#define CREATE_STRICT_TYPE(type, name) using name = aka::StrictType<type, struct name##TagType>;

namespace aka {

template <typename T, typename Tag>
class StrictType
{
private:
	typedef StrictType<T, Tag> ST;
public:
	using Type = T;
public:
	constexpr StrictType() : m_value(Type()) {}
	constexpr explicit StrictType(const Type value) : m_value(value) {}

	constexpr Type &value() { return m_value; }
	constexpr const Type &value() const { return m_value; }
	constexpr explicit operator Type& () { return m_value; }
	constexpr explicit operator const Type& () const { return m_value; }

	constexpr bool operator==(const ST& rhs) const { return m_value == rhs.m_value; }
	constexpr bool operator!=(const ST& rhs) const { return m_value != rhs.m_value; }
	constexpr bool operator<(const ST& rhs) const { return m_value < rhs.m_value; }
	constexpr bool operator>(const ST& rhs) const { return m_value > rhs.m_value; }
	constexpr bool operator<=(const ST& rhs) const { return m_value <= rhs.m_value; }
	constexpr bool operator>=(const ST& rhs) const { return m_value >= rhs.m_value; }

	constexpr ST operator+(const ST& rhs) const { return ST(m_value + rhs.m_value); }
	constexpr ST operator-(const ST& rhs) const { return ST(m_value - rhs.m_value); }
	constexpr ST operator/(const ST& rhs) const { return ST(m_value / rhs.m_value); }
	constexpr ST operator*(const ST& rhs) const { return ST(m_value * rhs.m_value); }
	constexpr ST& operator+=(const ST& rhs) { m_value += rhs.m_value; return *this; }
	constexpr ST& operator-=(const ST& rhs) { m_value -= rhs.m_value; return *this; }
	constexpr ST& operator/=(const ST& rhs) { m_value /= rhs.m_value; return *this; }
	constexpr ST& operator*=(const ST& rhs) { m_value *= rhs.m_value; return *this; }

	constexpr ST operator+(const Type& rhs) const { return ST(m_value + rhs); }
	constexpr ST operator-(const Type& rhs) const { return ST(m_value - rhs); }
	constexpr ST operator/(const Type& rhs) const { return ST(m_value / rhs); }
	constexpr ST operator*(const Type& rhs) const { return ST(m_value * rhs); }
	constexpr ST& operator+=(const Type& rhs) { m_value = m_value + rhs; return *this; }
	constexpr ST& operator-=(const Type& rhs) { m_value = m_value - rhs; return *this; }
	constexpr ST& operator/=(const Type& rhs) { m_value = m_value / rhs; return *this; }
	constexpr ST& operator*=(const Type& rhs) { m_value = m_value * rhs; return *this; }

	constexpr ST& operator++() { ++m_value;  return *this; }
	constexpr ST& operator--() { --m_value;  return *this; }
	constexpr ST operator++(int) { return ST(m_value++); }
	constexpr ST operator--(int) { return ST(m_value--); }

private:
	Type m_value;
};

template <class Type, class Tag>
std::ostream& operator<<(std::ostream& os, StrictType<Type, Tag> index) {
	os << index.value();
	return os;
}

};