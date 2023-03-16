#pragma once

#include <Aka/Core/Config.h>

#include <type_traits>

namespace aka {

template <typename T>
inline constexpr UnderlyingType<T> EnumToIntegral(T value)
{
	return static_cast<UnderlyingType<T>>(value);
}
// TODO use strict type ?
template <typename T>
inline constexpr UnderlyingType<T> EnumToIndex(T value)
{
	return static_cast<UnderlyingType<T>>(value) - EnumToIntegral<T>(T::First);
}

template <typename T>
inline constexpr T IndexToEnum(UnderlyingType<T> value)
{
	return static_cast<T>(value + EnumToIntegral<T>(T::First));
}

template <typename T>
inline constexpr UnderlyingType<T> EnumCount()
{
	return EnumToIntegral<T>(T::Last) - EnumToIntegral(T::First) + static_cast<UnderlyingType<T>>(1);
}

template <typename T>
class EnumIterator
{
	static_assert(std::is_enum<T>::value, "This is an enum iterator");
public:
	explicit EnumIterator(T value) : m_value(value) {}
	T& operator++() 
	{ 
		m_value = static_cast<T>(static_cast<UnderlyingType<T>>(m_value) + static_cast<UnderlyingType<T>>(1)); 
		return m_value; 
	}
	T operator++(int) 
	{ 
		T old = m_value; 
		operator++(m_value);
		return old; 
	}
	T operator*() { return m_value; }
	bool operator==(const EnumIterator<T>& value) { return value.m_value == m_value;  }
	bool operator!=(const EnumIterator<T>& value) { return value.m_value != m_value; }
private:
	T m_value;
};

template <typename T>
class EnumRange
{
public:
	EnumIterator<T> begin() const { return EnumIterator<T>(T::First); }
	EnumIterator<T> end() const { return EnumIterator<T>(T::Last); }
};

#define AKA_IMPLEMENT_BITMASK_OPERATOR(EnumClassType) \
inline EnumClassType operator&(EnumClassType lhs, EnumClassType rhs) { return static_cast<EnumClassType>(EnumToIntegral(lhs) & EnumToIntegral(rhs)); }  \
inline EnumClassType operator|(EnumClassType lhs, EnumClassType rhs) { return static_cast<EnumClassType>(EnumToIntegral(lhs) | EnumToIntegral(rhs)); } \
inline EnumClassType& operator|=(EnumClassType& lhs, EnumClassType rhs) { lhs = static_cast<EnumClassType>(EnumToIntegral(lhs) | EnumToIntegral(rhs)); return lhs; } \
inline EnumClassType& operator&=(EnumClassType& lhs, EnumClassType rhs) { lhs = static_cast<EnumClassType>(EnumToIntegral(lhs) & EnumToIntegral(rhs));  return lhs; } \
inline EnumClassType operator~(EnumClassType value) { return static_cast<EnumClassType>(~EnumToIntegral(value)); } \
inline bool has(EnumClassType mask, EnumClassType flag) { return (mask & flag) == flag; } \
inline bool isNull(EnumClassType mask) { return EnumToIntegral(mask) == 0; }
// TODO << >> ^ ~ in BITWISE instead of bitmask

};
