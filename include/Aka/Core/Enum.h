#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Core/BitField.h>

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
inline constexpr uint32_t EnumBitCount()
{
	return 31 - countLeadingZero(static_cast<uint32_t>(T::Last));
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

template<typename Type, typename Container = uint32_t>
struct EnumMask
{
	static_assert(std::is_enum<Type>::value, "This is an enum iterator");
	static_assert(EnumCount<Type>() <= CHAR_BIT * sizeof(Container));
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
};


#define AKA_IMPLEMENT_BITMASK_OPERATOR(EnumClassType) \
inline EnumClassType operator&(EnumClassType lhs, EnumClassType rhs) { return static_cast<EnumClassType>(static_cast<UnderlyingType<EnumClassType>>(lhs) & static_cast<UnderlyingType<EnumClassType>>(rhs)); }  \
inline EnumClassType operator|(EnumClassType lhs, EnumClassType rhs) { return static_cast<EnumClassType>(static_cast<UnderlyingType<EnumClassType>>(lhs) | static_cast<UnderlyingType<EnumClassType>>(rhs)); } \
inline EnumClassType& operator|=(EnumClassType& lhs, EnumClassType rhs) { lhs = static_cast<EnumClassType>(static_cast<UnderlyingType<EnumClassType>>(lhs) | static_cast<UnderlyingType<EnumClassType>>(rhs)); return lhs; } \
inline EnumClassType& operator&=(EnumClassType& lhs, EnumClassType rhs) { lhs = static_cast<EnumClassType>(static_cast<UnderlyingType<EnumClassType>>(lhs) & static_cast<UnderlyingType<EnumClassType>>(rhs));  return lhs; } \
inline EnumClassType operator~(EnumClassType value) { return static_cast<EnumClassType>(~static_cast<UnderlyingType<EnumClassType>>(value)); } \
inline bool has(EnumClassType mask, EnumClassType flag) { return (mask & flag) == flag; } \
inline bool asBool(EnumClassType mask) { return mask != (EnumClassType)0; } \
inline bool isNull(EnumClassType mask) { return static_cast<UnderlyingType<EnumClassType>>(mask) == 0; } \
inline UnderlyingType<EnumClassType> toMask(EnumClassType mask) { return static_cast<UnderlyingType<EnumClassType>>(mask); }
// TODO << >> ^ ~ in BITWISE instead of bitmask

};
