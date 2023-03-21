#pragma once

#include <Aka/Core/Config.h>

#include <type_traits>

namespace aka {

// An enum class in aka is expected to have this structure:
//	enum class Type : uint32_t 
// {
//		Unknown,
//		
//		MyValue0,
//		MyValue1,
//		MyValue2,
// 
//		First = MyValue0,
//		Last = MyValue2,
//	}
// If this structure is respected, it will work with all following helpers.
// This way, when zero initialized, the enum is invalid. To cast it as an index, use EnumToIndex

template <typename T>
inline constexpr UnderlyingType<T> EnumToIndex(T value)
{
	return static_cast<UnderlyingType<T>>(value) - static_cast<UnderlyingType<T>>(T::First);
}

template <typename T>
inline constexpr T IndexToEnum(UnderlyingType<T> value)
{
	return static_cast<UnderlyingType<T>>(value + static_cast<UnderlyingType<T>>(T::First));
}

template <typename T>
inline constexpr uint32_t EnumCount()
{
	return static_cast<uint32_t>(static_cast<UnderlyingType<T>>(T::Last) - static_cast<UnderlyingType<T>>(T::First) + static_cast<UnderlyingType<T>>(1));
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
	EnumIterator<T> end() const { return EnumIterator<T>(static_cast<T>(static_cast<UnderlyingType<T>>(T::Last) + static_cast<UnderlyingType<T>>(1))); }
};

#define AKA_IMPLEMENT_BITMASK_OPERATOR(EnumClassType) \
inline EnumClassType operator&(EnumClassType lhs, EnumClassType rhs) { return static_cast<EnumClassType>(static_cast<UnderlyingType<EnumClassType>>(lhs) & static_cast<UnderlyingType<EnumClassType>>(rhs)); }  \
inline EnumClassType operator|(EnumClassType lhs, EnumClassType rhs) { return static_cast<EnumClassType>(static_cast<UnderlyingType<EnumClassType>>(lhs) | static_cast<UnderlyingType<EnumClassType>>(rhs)); } \
inline EnumClassType& operator|=(EnumClassType& lhs, EnumClassType rhs) { lhs = static_cast<EnumClassType>(static_cast<UnderlyingType<EnumClassType>>(lhs) | static_cast<UnderlyingType<EnumClassType>>(rhs)); return lhs; } \
inline EnumClassType& operator&=(EnumClassType& lhs, EnumClassType rhs) { lhs = static_cast<EnumClassType>(static_cast<UnderlyingType<EnumClassType>>(lhs) & static_cast<UnderlyingType<EnumClassType>>(rhs));  return lhs; } \
inline EnumClassType operator~(EnumClassType value) { return static_cast<EnumClassType>(~static_cast<UnderlyingType<EnumClassType>>(value)); } \
inline bool has(EnumClassType mask, EnumClassType flag) { return (mask & flag) == flag; } \
inline bool isNull(EnumClassType mask) { return static_cast<UnderlyingType<EnumClassType>>(mask) == 0; }
// TODO << >> ^ ~ in BITWISE instead of bitmask

};
