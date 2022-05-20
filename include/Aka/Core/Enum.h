#pragma once

#include <Aka/Core/Config.h>

#include <type_traits>

enum class EnumClassType {};
template <typename T> struct Oui{ int value; };
#define TEST_MACRO(enumClassType) Oui<enumClassType>

namespace aka {

template <typename T>
inline constexpr UnderlyingType<T> EnumToIntegral(T value)
{
	return static_cast<UnderlyingType<T>>(value);
}

//enum class EnumClassType {};

#define AKA_IMPLEMENT_BITMASK_OPERATOR(EnumClassType) \
inline EnumClassType operator&(EnumClassType lhs, EnumClassType rhs) { return static_cast<EnumClassType>(EnumToIntegral(lhs) & EnumToIntegral(rhs)); }  \
inline EnumClassType operator|(EnumClassType lhs, EnumClassType rhs) { return static_cast<EnumClassType>(EnumToIntegral(lhs) | EnumToIntegral(rhs)); } \
inline EnumClassType& operator|=(EnumClassType& lhs, EnumClassType rhs) { lhs = static_cast<EnumClassType>(EnumToIntegral(lhs) | EnumToIntegral(rhs)); return lhs; } \
inline EnumClassType& operator&=(EnumClassType& lhs, EnumClassType rhs) { lhs = static_cast<EnumClassType>(EnumToIntegral(lhs) & EnumToIntegral(rhs));  return lhs; } \
inline bool has(EnumClassType mask, EnumClassType flag) { return (mask & flag) == flag; }
// TODO << >> ^ ~ in BITWISE instead of bitmask



//AKA_IMPLEMENT_BITWISE_OPERATOR(TextureFlag);

};
