#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Core/Bitmask.hpp>

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
//
// For bitmask, the structure should instead follow the All convention:
//	enum class Type : uint32_t 
// {
//		None,
//		
//		MyValue0 = 1 << 0,
//		MyValue1 = 1 << 1,
//		MyValue2 = 1 << 2,
// 
//		All = MyValue0 | MyValue1 | MyValue2,
//	}

template <typename T>
inline constexpr UnderlyingType<T> EnumToValue(T value)
{
	return static_cast<UnderlyingType<T>>(value);
}

template <typename T>
inline constexpr UnderlyingType<T> EnumToIndex(T value)
{
	return static_cast<UnderlyingType<T>>(value) - static_cast<UnderlyingType<T>>(T::First);
}

template <typename T>
inline constexpr T IndexToEnum(UnderlyingType<T> value)
{
	return static_cast<T>(value + static_cast<UnderlyingType<T>>(T::First));
}

template <typename T>
inline constexpr uint32_t EnumCount()
{
	return static_cast<uint32_t>(static_cast<UnderlyingType<T>>(T::Last) - static_cast<UnderlyingType<T>>(T::First) + static_cast<UnderlyingType<T>>(1));
}

template <typename T>
inline constexpr bool EnumIsInRange(T _value)
{
	return _value <= T::Last && _value >= T::First;
}

template <uint32_t Count>	constexpr uint32_t BitCountOf() { return 1U + BitCountOf<(Count >> 1U)>(); }
template <>					constexpr uint32_t BitCountOf<1>() { return 1U; }
template <>					constexpr uint32_t BitCountOf<0>() { return 0U; }
template <typename T>
inline constexpr uint32_t EnumBitCount()
{
	return BitCountOf<(static_cast<uint32_t>(T::Last))>();
}

template <typename T>
class EnumIterator
{
	static_assert(std::is_enum<T>::value, "This is an enum iterator");
public:
	explicit EnumIterator(T value) : m_value(value) {}
	EnumIterator<T>& operator++()
	{ 
		m_value = static_cast<T>(static_cast<UnderlyingType<T>>(m_value) + static_cast<UnderlyingType<T>>(1)); 
		return *this; 
	}
	EnumIterator<T> operator++(int) 
	{ 
		EnumIterator<T> old = *this;
		++(*this);
		return old; 
	}
	T& operator*() { return m_value; }
	const T& operator*() const { return m_value; }
	bool operator==(const EnumIterator<T>& value) const { return value.m_value == m_value;  }
	bool operator!=(const EnumIterator<T>& value) const { return value.m_value != m_value; }
private:
	T m_value;
};

template <typename T>
class EnumRange
{
public:
	EnumRange() : EnumRange(T::First, static_cast<T>(static_cast<UnderlyingType<T>>(T::Last) + static_cast<UnderlyingType<T>>(1))) {}
	explicit EnumRange(T _last) : EnumRange(T::First, _last) {}
	explicit EnumRange(T _first, T _last) : m_begin(_first), m_end(_last) {}
	EnumIterator<T> begin() const { return EnumIterator<T>(m_begin); }
	EnumIterator<T> end() const { return EnumIterator<T>(m_end); }
private:
	T m_begin, m_end;
};

template <typename Type>
struct EnumBitIterator
{
public:
	explicit EnumBitIterator(Type _mask) :
		m_mask(_mask),
		m_value(static_cast<Type>(1U << countTrailingZero(static_cast<uint64_t>(_mask))))
	{
		// Clear first value.
		m_mask &= ~m_value;
	}
	EnumBitIterator& operator++()
	{
		m_value = static_cast<Type>(1U << countTrailingZero(static_cast<uint64_t>(m_mask)));
		m_mask &= ~m_value;
		return *this;
	}
	EnumBitIterator operator++(int)
	{
		EnumBitIterator old = m_mask;
		++(*this);
		return old;
	}
	const Type& operator*() const { return m_value; }
	bool operator==(const EnumBitIterator& value) const { return value.m_value == m_value; }
	bool operator!=(const EnumBitIterator& value) const { return value.m_value != m_value; }
private:
	Type m_mask;
	Type m_value;
};

template <typename T>
class EnumBitRange
{
public:
	EnumBitRange() : EnumBitRange(
		T::All, 
		static_cast<T>(0)
	) {}
	explicit EnumBitRange(T _mask) : m_begin(_mask), m_end(static_cast<T>(0)) {}
	EnumBitIterator<T> begin() const { return EnumBitIterator<T>(m_begin); }
	EnumBitIterator<T> end() const { return EnumBitIterator<T>(m_end); }
private:
	T m_begin, m_end;
};

/*template<typename Type, typename Container = uint32_t>
struct EnumMask
{
	static_assert(std::is_enum<Type>::value, "This is an enum mask");
	static_assert(EnumCount<Type>() <= sizeof(Type) * CHAR_BIT );
public:
	EnumMask() = default;
	explicit EnumMask(Type _type) : m_mask(1U << EnumToIndex(_type)) {}
	explicit EnumMask(Container _container) : m_mask(_container) {}

	EnumMask& operator&=(Type _type) { m_mask &= 1U << EnumToIndex(_type); return *this; }
	EnumMask& operator|=(Type _type) { m_mask |= 1U << EnumToIndex(_type); return *this; }
	EnumMask& operator&=(EnumMask _mask) { m_mask &= _mask.m_mask; return *this; }
	EnumMask& operator|=(EnumMask _mask) { m_mask &= _mask.m_mask; return *this; }

	EnumMask operator&(Type _type) const { return EnumMask(m_mask & (1U << EnumToIndex(_type))); }
	EnumMask operator|(Type _type) const { return EnumMask(m_mask | (1U << EnumToIndex(_type))); }
	EnumMask operator&(EnumMask _mask) const { return EnumMask(m_mask & _mask.m_mask); }
	EnumMask operator|(EnumMask _mask) const { return EnumMask(m_mask | _mask.m_mask); }

	EnumMask operator~() const { return EnumMask(~m_mask); }

	bool operator==(const EnumMask& _value) { return _value.m_mask == m_mask; }
	bool operator!=(const EnumMask& _value) { return _value.m_mask != m_mask; }

	void clear() { m_mask = 0; }
	bool empty() const { return m_mask == 0; }
	bool has(Type _value) const { return (m_mask & (1U << EnumToIndex(_value))) != 0; }
	Container get() const { return m_mask; }

	struct Iterator
	{
	public:
		explicit Iterator(const EnumMask<Type>& _mask, Type value) :
			m_value(value),
			m_mask(_mask) 
		{
		}
		Iterator& operator++()
		{
			while (*(++m_value) < Type::Last)
				if ((m_mask & *m_value).has(*m_value))
					break;
			return *this;
		}
		Iterator operator++(int)
		{
			Iterator old = m_value;
			++(*this);
			return old;
		}
		const Type& operator*() const { return *m_value; }
		bool operator==(const Iterator& value) const { return value.m_value == m_value; }
		bool operator!=(const Iterator& value) const { return value.m_value != m_value; }
	private:
		EnumIterator<Type> m_value;
		const EnumMask<Type>& m_mask;
	};

	Iterator begin() { return Iterator(*this, IndexToEnum<Type>(countTrailingZero(m_mask))); }
	Iterator end() { return Iterator(*this, Type::Last); }
private:
	Container m_mask = 0;
};*/


#define AKA_IMPLEMENT_BITMASK_OPERATOR(EnumClassType) \
inline constexpr EnumClassType operator&(EnumClassType lhs, EnumClassType rhs) { return static_cast<EnumClassType>(static_cast<::aka::UnderlyingType<EnumClassType>>(lhs) & static_cast<::aka::UnderlyingType<EnumClassType>>(rhs)); } \
inline constexpr EnumClassType operator|(EnumClassType lhs, EnumClassType rhs) { return static_cast<EnumClassType>(static_cast<::aka::UnderlyingType<EnumClassType>>(lhs) | static_cast<::aka::UnderlyingType<EnumClassType>>(rhs)); } \
inline constexpr EnumClassType& operator|=(EnumClassType& lhs, EnumClassType rhs) { lhs = static_cast<EnumClassType>(static_cast<::aka::UnderlyingType<EnumClassType>>(lhs) | static_cast<::aka::UnderlyingType<EnumClassType>>(rhs)); return lhs; } \
inline constexpr EnumClassType& operator&=(EnumClassType& lhs, EnumClassType rhs) { lhs = static_cast<EnumClassType>(static_cast<::aka::UnderlyingType<EnumClassType>>(lhs) & static_cast<::aka::UnderlyingType<EnumClassType>>(rhs)); return lhs; } \
inline constexpr EnumClassType operator~(EnumClassType value) { return static_cast<EnumClassType>(~static_cast<::aka::UnderlyingType<EnumClassType>>(value)); } \
inline constexpr bool has(EnumClassType mask, EnumClassType flag) { return (mask & flag) == flag; } \
inline constexpr bool asBool(EnumClassType mask) { return mask != (EnumClassType)0; } \
inline constexpr bool isNull(EnumClassType mask) { return static_cast<::aka::UnderlyingType<EnumClassType>>(mask) == 0; } \
inline constexpr ::aka::UnderlyingType<EnumClassType> toMask(EnumClassType mask) { return static_cast<::aka::UnderlyingType<EnumClassType>>(mask); }
// TODO << >> ^ ~ in BITWISE instead of bitmask

};
