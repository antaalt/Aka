#pragma once

#include <iostream>

#define CREATE_STRICT_TYPE(type, name) using name = aka::StrictType<type, struct name##TagType>;

namespace aka {

template <typename Type, typename Tag>
class StrictType
{
private:
	typedef StrictType<Type, Tag> ST;
public:
	constexpr StrictType() : m_value(Type()) {}
	constexpr explicit StrictType(const Type value) : m_value(value) {}

	constexpr Type &value() { return m_value; }
	constexpr const Type &value() const { return m_value; }
	constexpr Type &operator()() { return m_value; }
	constexpr const Type &operator()() const { return m_value; }

	constexpr bool operator==(const ST& rhs) const { return m_value == rhs.m_value; }
	constexpr bool operator!=(const ST& rhs) const { return m_value != rhs.m_value; }
	constexpr bool operator<(const ST& rhs) const { return m_value < rhs.m_value; }
	constexpr bool operator>(const ST& rhs) const { return m_value > rhs.m_value; }
	constexpr bool operator<=(const ST& rhs) const { return m_value <= rhs.m_value; }
	constexpr bool operator>=(const ST& rhs) const { return m_value >= rhs.m_value; }
	constexpr ST operator+(const ST& rhs) const { return m_value + rhs.m_value; }
	constexpr ST operator-(const ST& rhs) const { return m_value - rhs.m_value; }
	constexpr ST& operator+=(const ST& rhs) const { m_value += rhs.m_value; return *this; }
	constexpr ST& operator-=(const ST& rhs) const { m_value -= rhs.m_value; return *this; }

	ST& operator+=(const ST& rhs) { m_value = m_value + rhs; return *this; }
	ST& operator-=(const ST& rhs) { m_value = m_value - rhs; return *this; }
	Type& operator++() { m_value++;  return *this; }
	Type& operator--() { m_value--;  return *this; }
	Type operator++(int) { const Type ret(m_value); m_value++; return ret; }
	Type operator--(int) { const Type ret(m_value); m_value--; return ret; }

private:
	Type m_value;
};

template <class Type, class Tag>
std::ostream& operator<<(std::ostream& os, StrictType<Type, Tag> index) {
	os << index.value();
	return os;
}

};