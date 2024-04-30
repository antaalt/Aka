#pragma once

#include <stdint.h>
#include <iostream>
#include <sstream>

#include <Aka/Core/Config.h>
#include <Aka/Core/Hash.hpp>
#include <Aka/Core/Geometry.h>
#include <Aka/Core/Container/Vector.h>

namespace aka {

template <typename T = char, AllocatorCategory Category = AllocatorCategory::Global>
class Str final
{
	static const size_t defaultCapacity = 16;
public:
	using Char = T;
	static const size_t invalid = -1;
public:
	Str();
	Str(const T* string);
	explicit Str(Allocator& allocator);
	explicit Str(const T* string, Allocator& allocator);
	explicit Str(const T* string, size_t length, Allocator& allocator = mem::getAllocator(AllocatorMemoryType::String, Category));
	explicit Str(size_t length, T character, Allocator& allocator = mem::getAllocator(AllocatorMemoryType::String, Category));
	explicit Str(size_t length, Allocator& allocator = mem::getAllocator(AllocatorMemoryType::String, Category));
	Str(const Str& string, Allocator& allocator = mem::getAllocator(AllocatorMemoryType::String, Category));
	Str(Str&& string, Allocator& allocator = mem::getAllocator(AllocatorMemoryType::String, Category));
	Str& operator=(const Str& string);
	Str& operator=(Str&& string);
	Str& operator=(const T* string);
	~Str();

	T& operator[](size_t index);
	const T& operator[](size_t index) const;

	bool operator==(const Str& string) const;
	bool operator!=(const Str& string) const;
	bool operator<(const Str& string) const;
	bool operator>(const Str& string) const;
	bool operator<=(const Str& string) const;
	bool operator>=(const Str& string) const;
	bool operator==(const T* string) const;
	bool operator!=(const T* string) const;

	Str& operator+=(const Str& string);
	Str& operator+=(const T* string);
	Str& operator+=(T character);
	Str operator+(const Str& string) const;
	Str operator+(const T* string) const;
	Str operator+(T character) const;

	Str& append(const Str& string);
	Str& append(const T* string, size_t length);
	Str& append(const T* string);
	Str& append(T character);

	// Get the size of the string
	size_t length() const;
	// Get the size of the string
	size_t size() const;
	// Get the capacity of the string
	size_t capacity() const;
	// Resize the string
	void resize(size_t length);
	// Resize the string capacity
	void reserve(size_t size);
	// Empty the string
	void clear();
	// Check if string empty
	bool empty() const;
	// Pointer to the string
	T* cstr();
	// Pointer to the string
	const T* cstr() const;
	// Pointer to the string
	T* data();
	// Pointer to the string
	const T* data() const;
	// Last character of string
	T last() const;
	// Pointer to beginning of string
	T* begin();
	// Pointer to ending of string
	T* end();
	// Pointer to beginning of string
	const T* begin() const;
	// Pointer to ending of string
	const T* end() const;
	
	// Get a null raw string
	static const T* null();
	// Copy a raw string in another one
	static T* copy(T* dst, size_t dstSize, const T* src);
	// Get the length of a raw string
	static size_t length(const T* string);
	// Compare a raw string with another one
	static int compare(const T* lhs, const T* rhs);
	// Format a raw string
	template <typename ...Args>
	static Str format(const T* string, Args ...args);
	// Format a string
	template<typename U, typename ...Args>
	static Str from(U t, Args ...args);
	// Format a string
	template<typename U>
	static Str from(U value);

	// Find a character and return its position
	size_t find(T character, size_t offset = 0) const;
	// Find the first occurence of the character and return its position
	size_t findFirst(T character, size_t offset = 0) const;
	// Find the last occurence of the character and return its position
	size_t findLast(T character, size_t offset = 0) const;
	// Find a character and split the string
	Vector<Str> split(T c) const;
	// Create a new string from start to end of string
	Str substr(size_t start) const;
	// Create a new string from start to end
	Str substr(size_t start, size_t len) const;

private:
	Allocator& m_allocator;
	T* m_string;
	size_t m_length;
	size_t m_capacity;
};

using String = Str<char>;
using StringWide = Str<wchar_t>;
using String16 = Str<char16_t>;
using String32 = Str<char32_t>;

template<typename T, AllocatorCategory Category>
inline Str<T, Category>::Str() :
	Str<T, Category>(Str<T, Category>::null(), 0)
{
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>::Str(const T* string) :
	Str<T, Category>(string, Str<T, Category>::length(string))
{
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>::Str(Allocator& allocator) :
	Str<T, Category>(Str<T, Category>::null(), 0, allocator)
{
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>::Str(const T* str, Allocator& allocator) :
	Str<T, Category>(str, Str<T, Category>::length(str), allocator)
{
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>::Str(const T* str, size_t length, Allocator& allocator) :
	m_allocator(allocator),
	m_string(m_allocator.allocate<T>(max(length + 1, defaultCapacity))),
	m_length(length),
	m_capacity(max(length + 1, defaultCapacity))
{
	Str<T, Category>::copy(m_string, m_length, str);
	m_string[length] = '\0';
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>::Str(size_t length, T character, Allocator& allocator) :
	m_allocator(allocator),
	m_string(m_allocator.allocate<T>(max(length + 1, defaultCapacity))),
	m_length(length),
	m_capacity(max(length + 1, defaultCapacity))
{
	for (size_t i = 0; i < length; i++)
		m_string[i] = character;
	m_string[length] = '\0';
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>::Str(size_t length, Allocator& allocator) :
	m_allocator(allocator),
	m_string(m_allocator.allocate<T>(max(length + 1, defaultCapacity))),
	m_length(length),
	m_capacity(max(length + 1, defaultCapacity))
{
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>::Str(const Str<T, Category>& str, Allocator& allocator) :
	Str<T, Category>(str.m_string, str.m_length)
{
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>::Str(Str<T, Category>&& string, Allocator& allocator) :
	m_allocator(allocator),
	m_string(nullptr),
	m_length(0),
	m_capacity(0)
{
	//std::swap(m_allocator, string.m_allocator);
	std::swap(m_string, string.m_string);
	std::swap(m_length, string.m_length);
	std::swap(m_capacity, string.m_capacity);
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>& Str<T, Category>::operator=(const Str<T, Category>& str)
{
	resize(str.m_length);
	Str<T, Category>::copy(m_string, str.m_length, str.m_string);
	return *this;
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>& Str<T, Category>::operator=(Str<T, Category>&& str)
{
	//std::swap(m_allocator, str.m_allocator);
	std::swap(m_string, str.m_string);
	std::swap(m_length, str.m_length);
	std::swap(m_capacity, str.m_capacity);
	return *this;
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>& Str<T, Category>::operator=(const T* str)
{
	size_t length = Str<T, Category>::length(str);
	resize(length);
	Str<T, Category>::copy(m_string, length, str);
	return *this;
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>::~Str()
{
	m_allocator.deallocate(m_string);
}
template<typename T, AllocatorCategory Category>
inline T& Str<T, Category>::operator[](size_t index)
{
	AKA_ASSERT(index <= m_length, "Out of range");
	return m_string[index];
}
template<typename T, AllocatorCategory Category>
inline const T& Str<T, Category>::operator[](size_t index) const
{
	AKA_ASSERT(index <= m_length, "Out of range");
	return m_string[index];
}
template<typename T, AllocatorCategory Category>
inline bool Str<T, Category>::operator==(const Str<T, Category>& str) const
{
	return Str<T, Category>::compare(m_string, str.m_string) == 0;
}
template<typename T, AllocatorCategory Category>
inline bool Str<T, Category>::operator!=(const Str<T, Category>& str) const
{
	return Str<T, Category>::compare(m_string, str.m_string) != 0;
}
template<typename T, AllocatorCategory Category>
inline bool Str<T, Category>::operator<(const Str<T, Category>& str) const
{
	return std::lexicographical_compare(begin(), end(), str.begin(), str.end());
}
template<typename T, AllocatorCategory Category>
inline bool Str<T, Category>::operator>(const Str<T, Category>& str) const
{
	return str < *this;
}
template<typename T, AllocatorCategory Category>
inline bool Str<T, Category>::operator<=(const Str<T, Category>& str) const
{
	return !(str < *this);
}
template<typename T, AllocatorCategory Category>
inline bool Str<T, Category>::operator>=(const Str<T, Category>& str) const
{
	return !(*this < str);
}
template<typename T, AllocatorCategory Category>
inline bool Str<T, Category>::operator==(const T* str) const
{
	return Str<T, Category>::compare(m_string, str) == 0;
}
template<typename T, AllocatorCategory Category>
inline bool Str<T, Category>::operator!=(const T* str) const
{
	return Str<T, Category>::compare(m_string, str) != 0;
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>& Str<T, Category>::operator+=(const Str<T, Category>& str)
{
	return append(str);
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>& Str<T, Category>::operator+=(const T* str)
{
	return append(str);
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>& Str<T, Category>::operator+=(T c)
{
	return append(c);
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category> Str<T, Category>::operator+(const Str<T, Category>& str) const
{
	Str<T, Category> out(*this);
	out.append(str);
	return out;
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category> Str<T, Category>::operator+(const T* str) const
{
	Str<T, Category> out(*this);
	out.append(str);
	return out;
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category> Str<T, Category>::operator+(T c) const
{
	Str<T, Category> out(*this);
	out.append(c);
	return out;
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>& Str<T, Category>::append(const Str<T, Category>& str)
{
	size_t off = m_length;
	size_t len = str.length();
	resize(off + len);
	Str<T, Category>::copy(m_string + off, len, str.cstr());
	return *this;
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>& Str<T, Category>::append(const T* str, size_t length)
{
	size_t off = m_length;
	size_t len = length;
	resize(off + len);
	Str<T, Category>::copy(m_string + off, len, str);
	m_string[m_length] = '\0';
	return *this;
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>& Str<T, Category>::append(const T* str)
{
	size_t off = m_length;
	size_t len = Str<T, Category>::length(str);
	resize(off + len);
	Str<T, Category>::copy(m_string + off, len, str);
	m_string[m_length] = '\0';
	return *this;
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category>& Str<T, Category>::append(T c)
{
	size_t len = m_length + 1;
	resize(len);
	m_string[len - 1] = c;
	m_string[len] = '\0';
	return *this;
}
template<typename T, AllocatorCategory Category>
inline size_t Str<T, Category>::length() const
{
	return m_length;
}
template<typename T, AllocatorCategory Category>
inline size_t Str<T, Category>::size() const
{
	return m_length;
}
template<typename T, AllocatorCategory Category>
inline size_t Str<T, Category>::capacity() const
{
	return m_capacity;
}
template<typename T, AllocatorCategory Category>
inline void Str<T, Category>::resize(size_t length)
{
	if (m_length == length)
		return;
	reserve(length + 1);
	m_length = length;
}
template<typename T, AllocatorCategory Category>
inline void Str<T, Category>::reserve(size_t size)
{
	if (size <= m_capacity)
		return;
	size_t oldCapacity = m_capacity;
	size_t newCapacity = max(size, defaultCapacity);
	T* buffer = m_allocator.allocate<T>(newCapacity);
	Str<T, Category>::copy(buffer, oldCapacity, m_string);
	m_allocator.deallocate(m_string);
	m_capacity = newCapacity;
	m_string = buffer;
}
template<typename T, AllocatorCategory Category>
inline void Str<T, Category>::clear()
{
	m_length = 0;
	m_string[0] = '\0';
}
template<typename T, AllocatorCategory Category>
inline bool Str<T, Category>::empty() const
{
	return m_length == 0;
}
template<typename T, AllocatorCategory Category>
inline T* Str<T, Category>::cstr()
{
	return m_string;
}
template<typename T, AllocatorCategory Category>
inline const T* Str<T, Category>::cstr() const
{
	return m_string;
}
template<typename T, AllocatorCategory Category>
inline T* Str<T, Category>::data()
{
	return m_string;
}
template<typename T, AllocatorCategory Category>
inline const T* Str<T, Category>::data() const
{
	return m_string;
}
template<typename T, AllocatorCategory Category>
inline T Str<T, Category>::last() const
{
	return m_string[m_length - 1];
}
template<typename T, AllocatorCategory Category>
inline T* Str<T, Category>::begin()
{
	return m_string;
}
template<typename T, AllocatorCategory Category>
inline T* Str<T, Category>::end()
{
	return m_string + m_length;
}
template<typename T, AllocatorCategory Category>
inline const T* Str<T, Category>::begin() const
{
	return m_string;
}
template<typename T, AllocatorCategory Category>
inline const T* Str<T, Category>::end() const
{
	return m_string + m_length;
}
template<typename T, AllocatorCategory Category>
inline T* Str<T, Category>::copy(T* dst, size_t count, const T* src)
{
	size_t size = 0;
	T* tmp = dst;
	while ((*tmp++ = *src++) != 0 && size++ < count);
	return dst;
}
template<typename T, AllocatorCategory Category>
inline size_t Str<T, Category>::length(const T* string)
{
	if (string == nullptr) return 0;
	const T* tmp = string;
	for (; (*tmp) != 0; ++tmp);
	return tmp - string;
}
template<typename T, AllocatorCategory Category>
inline int Str<T, Category>::compare(const T* lhs, const T* rhs)
{
	for (; *lhs && (*lhs == *rhs); lhs++, rhs++) {}
	return (unsigned char)(*lhs) - (unsigned char)(*rhs);
}
template<typename T, AllocatorCategory Category>
template<typename ...Args>
inline Str<T, Category> Str<T, Category>::format(const T* string, Args ...args)
{
	if constexpr (std::is_same<char, T>::value)
	{
		int size = snprintf(nullptr, 0, string, args...);
		Str<char, Category> formattedString(size);
		int newSize = snprintf(formattedString.cstr(), size + 1, string, args...);
		AKA_ASSERT(newSize == size, "Invalid formatting");
		return formattedString;
	}
	else if constexpr (std::is_same<wchar_t, T>::value)
	{
		int size = snprintf(nullptr, 0, string, args...);
		Str<wchar_t, Category> formattedString(size);
		int newSize = swprintf(formattedString.cstr(), size + 1, string, args...);
		AKA_ASSERT(newSize == size, "Invalid formatting");
		return formattedString;
	}
	else
	{
		AKA_NOT_IMPLEMENTED;
	}
}
template<typename T, AllocatorCategory Category>
inline size_t Str<T, Category>::find(T character, size_t offset) const
{
	return findFirst(character, offset);
}
template<typename T, AllocatorCategory Category>
inline size_t Str<T, Category>::findFirst(T character, size_t offset) const
{
	for (size_t i = offset; i < m_length; ++i)
		if (character == m_string[i])
			return i;
	return invalid;
}
template<typename T, AllocatorCategory Category>
inline size_t Str<T, Category>::findLast(T character, size_t offset) const
{
	if (m_length == 0)
		return invalid;
	for (size_t i = m_length - 1; i >= 1; --i)
		if (character == m_string[i])
			return i;
	return invalid;
}

template<typename T, AllocatorCategory Category>
template<typename U, typename ...Args>
inline Str<T, Category> Str<T, Category>::from(U t, Args ...args)
{
	return Str<T, Category>::from(t) + Str<T, Category>::from(args...);
}

template<typename T, AllocatorCategory Category>
template<typename U>
inline Str<T, Category> Str<T, Category>::from(U value)
{
	std::basic_stringstream<T> sstr;
	sstr << value;
	std::string str = sstr.str();
	return Str<T, Category>(str.c_str(), str.size());
}

template<typename T, AllocatorCategory Category>
inline Vector<Str<T, Category>> Str<T, Category>::split(T c) const
{
	size_t offset = 0;
	Vector<Str<T, Category>> strings;
	for (size_t i = 0; i < m_length; ++i)
	{
		// TODO handle multiple identical characters following.
		if (m_string[i] == c && offset != i)
		{
			strings.append(substr(offset, i - offset));
			offset = i + 1;
		}
	}
	// Add last element if not empty
	if (m_length > 0)
		strings.append(substr(offset, m_length - offset));
	return strings;
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category> Str<T, Category>::substr(size_t start) const
{
	return substr(start, m_length - start);
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category> Str<T, Category>::substr(size_t start, size_t len) const
{
	Str<T, Category> str(len);
	Str<T, Category>::copy(str.m_string, len, m_string + start);
	str[len] = '\0';
	return str;
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category> operator+(T c, const Str<T, Category>& str)
{
	return Str<T, Category>(1, c) + str;
}
template<typename T, AllocatorCategory Category>
inline Str<T, Category> operator+(const T* s, const Str<T, Category>& str)
{ 
	return Str<T, Category>(s) + str;
}
template<typename T, AllocatorCategory Category>
std::ostream& operator<<(std::ostream& os, const Str<T, Category>& str)
{
	os << str.cstr();
	return os;
}

template<typename T, AllocatorCategory Category>
const T* Str<T, Category>::null()
{
	if constexpr (std::is_same<char, T>::value)
	{
		return "";
	}
	else if constexpr (std::is_same<wchar_t, T>::value)
	{
		return L"";
	}
	else if constexpr (std::is_same<char16_t, T>::value)
	{
		return u"";
	}
	else if constexpr (std::is_same<char32_t, T>::value)
	{
		return U"";
	}
}

};

template <>
struct std::hash<aka::String>
{
	size_t operator()(const aka::String& string) const
	{
		return aka::hash::fnv(string.cstr(), string.length());
	}
};