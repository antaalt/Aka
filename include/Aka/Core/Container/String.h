#pragma once

#include <stdint.h>
#include <iostream>
#include <sstream>

#include <Aka/Core/Config.h>
#include <Aka/Core/Hash.hpp>
#include <Aka/Core/Geometry.h>
#include <Aka/Core/Container/Vector.h>

namespace aka {

template <typename T = char>
class Str final
{
	using AllocatorType = Allocator;
	static const size_t defaultCapacity = 16;
public:
	using Char = T;
	static const size_t invalid = -1;
public:
	Str();
	Str(const T* string);
	explicit Str(AllocatorType& allocator);
	explicit Str(const T* string, AllocatorType& allocator);
	explicit Str(const T* string, size_t length, AllocatorType& allocator = mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::String));
	explicit Str(size_t length, T character, AllocatorType& allocator = mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::String));
	explicit Str(size_t length, AllocatorType& allocator = mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::String));
	Str(const Str& string, AllocatorType& allocator = mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::String));
	Str(Str&& string, AllocatorType& allocator = mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::String));
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
	AllocatorType& m_allocator;
	T* m_string;
	size_t m_length;
	size_t m_capacity;
};

using String = Str<char>;
using StringWide = Str<wchar_t>;
using String16 = Str<char16_t>;
using String32 = Str<char32_t>;

template<typename T>
inline Str<T>::Str() :
	Str<T>(Str<T>::null(), 0)
{
}
template<typename T>
inline Str<T>::Str(const T* string) :
	Str<T>(string, Str<T>::length(string))
{
}
template<typename T>
inline Str<T>::Str(AllocatorType& allocator) :
	Str<T>(Str<T>::null(), 0, allocator)
{
}
template<typename T>
inline Str<T>::Str(const T* str, AllocatorType& allocator) :
	Str<T>(str, Str<T>::length(str), allocator)
{
}
template<typename T>
inline Str<T>::Str(const T* str, size_t length, AllocatorType& allocator) :
	m_allocator(allocator),
	m_string(static_cast<T*>(m_allocator.allocate(sizeof(T) * max(length + 1, defaultCapacity)))),
	m_length(length),
	m_capacity(max(length + 1, defaultCapacity))
{
	Str<T>::copy(m_string, m_capacity, str);
	m_string[length] = '\0';
}
template<typename T>
inline Str<T>::Str(size_t length, T character, AllocatorType& allocator) :
	m_allocator(allocator),
	m_string(static_cast<T*>(m_allocator.allocate(sizeof(T)* max(length + 1, defaultCapacity)))),
	m_length(length),
	m_capacity(max(length + 1, defaultCapacity))
{
	for (size_t i = 0; i < length; i++)
		m_string[i] = character;
	m_string[length] = '\0';
}
template<typename T>
inline Str<T>::Str(size_t length, AllocatorType& allocator) :
	m_allocator(allocator),
	m_string(static_cast<T*>(m_allocator.allocate(sizeof(T)* max(length + 1, defaultCapacity)))),
	m_length(length),
	m_capacity(max(length + 1, defaultCapacity))
{
}
template<typename T>
inline Str<T>::Str(const Str<T>& str, AllocatorType& allocator) :
	Str<T>(str.m_string, str.m_length)
{
}
template<typename T>
inline Str<T>::Str(Str<T>&& string, AllocatorType& allocator) :
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
template<typename T>
inline Str<T>& Str<T>::operator=(const Str<T>& str)
{
	resize(str.m_length);
	Str<T>::copy(m_string, str.m_capacity, str.m_string);
	return *this;
}
template<typename T>
inline Str<T>& Str<T>::operator=(Str<T>&& str)
{
	//std::swap(m_allocator, str.m_allocator);
	std::swap(m_string, str.m_string);
	std::swap(m_length, str.m_length);
	std::swap(m_capacity, str.m_capacity);
	return *this;
}
template<typename T>
inline Str<T>& Str<T>::operator=(const T* str)
{
	size_t length = Str<T>::length(str);
	resize(length);
	Str<T>::copy(m_string, length + 1, str);
	return *this;
}
template<typename T>
inline Str<T>::~Str()
{
	m_allocator.deallocate(m_string, sizeof(T) * m_capacity);
}
template<typename T>
inline T& Str<T>::operator[](size_t index)
{
	AKA_ASSERT(index <= m_length, "Out of range");
	return m_string[index];
}
template<typename T>
inline const T& Str<T>::operator[](size_t index) const
{
	AKA_ASSERT(index <= m_length, "Out of range");
	return m_string[index];
}
template<typename T>
inline bool Str<T>::operator==(const Str<T>& str) const
{
	return Str<T>::compare(m_string, str.m_string) == 0;
}
template<typename T>
inline bool Str<T>::operator!=(const Str<T>& str) const
{
	return Str<T>::compare(m_string, str.m_string) != 0;
}
template<typename T>
inline bool Str<T>::operator<(const Str<T>& str) const
{
	return std::lexicographical_compare(begin(), end(), str.begin(), str.end());
}
template<typename T>
inline bool Str<T>::operator>(const Str<T>& str) const
{
	return str < *this;
}
template<typename T>
inline bool Str<T>::operator<=(const Str<T>& str) const
{
	return !(str < *this);
}
template<typename T>
inline bool Str<T>::operator>=(const Str<T>& str) const
{
	return !(*this < str);
}
template<typename T>
inline bool Str<T>::operator==(const T* str) const
{
	return Str<T>::compare(m_string, str) == 0;
}
template<typename T>
inline bool Str<T>::operator!=(const T* str) const
{
	return Str<T>::compare(m_string, str) != 0;
}
template<typename T>
inline Str<T>& Str<T>::operator+=(const Str<T>& str)
{
	return append(str);
}
template<typename T>
inline Str<T>& Str<T>::operator+=(const T* str)
{
	return append(str);
}
template<typename T>
inline Str<T>& Str<T>::operator+=(T c)
{
	return append(c);
}
template<typename T>
inline Str<T> Str<T>::operator+(const Str<T>& str) const
{
	Str<T> out(*this);
	out.append(str);
	return out;
}
template<typename T>
inline Str<T> Str<T>::operator+(const T* str) const
{
	Str<T> out(*this);
	out.append(str);
	return out;
}
template<typename T>
inline Str<T> Str<T>::operator+(T c) const
{
	Str<T> out(*this);
	out.append(c);
	return out;
}
template<typename T>
inline Str<T>& Str<T>::append(const Str<T>& str)
{
	size_t off = m_length;
	size_t len = str.length();
	resize(off + len);
	Str<T>::copy(m_string + off, len + 1, str.cstr());
	return *this;
}
template<typename T>
inline Str<T>& Str<T>::append(const T* str, size_t length)
{
	size_t off = m_length;
	size_t len = length;
	resize(off + len);
	Str<T>::copy(m_string + off, len, str);
	m_string[m_length] = '\0';
	return *this;
}
template<typename T>
inline Str<T>& Str<T>::append(const T* str)
{
	size_t off = m_length;
	size_t len = Str<T>::length(str);
	resize(off + len);
	Str<T>::copy(m_string + off, len, str);
	m_string[m_length] = '\0';
	return *this;
}
template<typename T>
inline Str<T>& Str<T>::append(T c)
{
	size_t len = m_length + 1;
	resize(len);
	m_string[len - 1] = c;
	m_string[len] = '\0';
	return *this;
}
template<typename T>
inline size_t Str<T>::length() const
{
	return m_length;
}
template<typename T>
inline size_t Str<T>::size() const
{
	return m_length;
}
template<typename T>
inline size_t Str<T>::capacity() const
{
	return m_capacity;
}
template<typename T>
inline void Str<T>::resize(size_t length)
{
	if (m_length == length)
		return;
	reserve(length + 1);
	m_length = length;
}
template<typename T>
inline void Str<T>::reserve(size_t size)
{
	if (size <= m_capacity)
		return;
	size_t oldCapacity = m_capacity;
	T* buffer = static_cast<T*>(m_allocator.allocate(sizeof(T) * size));
	Str<T>::copy(buffer, oldCapacity, m_string);
	m_allocator.deallocate(m_string, sizeof(T) * oldCapacity);
	m_capacity = size;
	m_string = buffer;
}
template<typename T>
inline void Str<T>::clear()
{
	*this = Str<T>(Str<T>::null());
}
template<typename T>
inline bool Str<T>::empty() const
{
	return m_length == 0;
}
template<typename T>
inline T* Str<T>::cstr()
{
	return m_string;
}
template<typename T>
inline const T* Str<T>::cstr() const
{
	return m_string;
}
template<typename T>
inline T* Str<T>::data()
{
	return m_string;
}
template<typename T>
inline const T* Str<T>::data() const
{
	return m_string;
}
template<typename T>
inline T Str<T>::last() const
{
	return m_string[m_length - 1];
}
template<typename T>
inline T* Str<T>::begin()
{
	return m_string;
}
template<typename T>
inline T* Str<T>::end()
{
	return m_string + m_length;
}
template<typename T>
inline const T* Str<T>::begin() const
{
	return m_string;
}
template<typename T>
inline const T* Str<T>::end() const
{
	return m_string + m_length;
}
template <typename T>
inline T* Str<T>::copy(T* dst, size_t count, const T* src)
{
	size_t size = 0;
	T* tmp = dst;
	while ((*tmp++ = *src++) != 0 && size++ < count);
	return dst;
}
template <typename T>
inline size_t Str<T>::length(const T* string)
{
	if (string == nullptr) return 0;
	const T* tmp = string;
	for (; (*tmp) != 0; ++tmp);
	return tmp - string;
}
template <typename T>
inline int Str<T>::compare(const T* lhs, const T* rhs)
{
	for (; *lhs && (*lhs == *rhs); lhs++, rhs++) {}
	return (unsigned char)(*lhs) - (unsigned char)(*rhs);
}
template <>
template <typename ...Args>
inline Str<char> Str<char>::format(const char* string, Args ...args)
{
	int size = snprintf(nullptr, 0, string, args...);
	String formattedString(size);
	int newSize = snprintf(formattedString.cstr(), size + 1, string, args...);
	AKA_ASSERT(newSize == size, "Invalid formatting");
	return formattedString;
}
template <>
template <typename ...Args>
inline Str<wchar_t> Str<wchar_t>::format(const wchar_t* string, Args ...args)
{
	int size = snprintf(nullptr, 0, string, args...);
	StringWide formattedString(size);
	int newSize = swprintf(formattedString.cstr(), size + 1, string, args...);
	AKA_ASSERT(newSize == size, "Invalid formatting");
	return formattedString;
}
template <>
template <typename ...Args>
inline Str<char16_t> Str<char16_t>::format(const char16_t* string, Args ...args)
{
	AKA_NOT_IMPLEMENTED;
	return String16();
}
template <>
template <typename ...Args>
inline Str<char32_t> Str<char32_t>::format(const char32_t* string, Args ...args)
{
	AKA_NOT_IMPLEMENTED;
	return String32();
}
template <typename T>
inline size_t Str<T>::find(T character, size_t offset) const
{
	return findFirst(character, offset);
}
template <typename T>
inline size_t Str<T>::findFirst(T character, size_t offset) const
{
	for (size_t i = offset; i < m_length; ++i)
		if (character == m_string[i])
			return i;
	return invalid;
}
template <typename T>
inline size_t Str<T>::findLast(T character, size_t offset) const
{
	if (m_length == 0)
		return invalid;
	for (size_t i = m_length - 1; i >= 1; --i)
		if (character == m_string[i])
			return i;
	return invalid;
}

template<typename T>
template<typename U, typename ...Args>
inline Str<T> Str<T>::from(U t, Args ...args)
{
	return Str<T>::from(t) + Str<T>::from(args...);
}

template<typename T>
template<typename U>
inline Str<T> Str<T>::from(U value)
{
	std::basic_stringstream<T> sstr;
	sstr << value;
	std::string str = sstr.str();
	return Str<T>(str.c_str(), str.size());
}

template <typename T>
inline Vector<Str<T>> Str<T>::split(T c) const
{
	size_t offset = 0;
	Vector<Str<T>> strings;
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
template <typename T>
inline Str<T> Str<T>::substr(size_t start) const
{
	return substr(start, m_length - start);
}
template <typename T>
inline Str<T> Str<T>::substr(size_t start, size_t len) const
{
	Str<T> str(len);
	Str<T>::copy(str.m_string, len, m_string + start);
	str[len] = '\0';
	return str;
}
template <typename T>
inline Str<T> operator+(T c, const Str<T>& str)
{
	return Str<T>().append(c) + str;
}
template <typename T>
inline Str<T> operator+(const T* s, const Str<T>& str)
{ 
	return Str<T>(s) + str;
}
template <typename T>
std::ostream& operator<<(std::ostream& os, const Str<T>& str)
{
	os << str.cstr();
	return os;
}

template <> const char* Str<char>::null();
template <> const wchar_t* Str<wchar_t>::null();
template <> const char16_t* Str<char16_t>::null();
template <> const char32_t* Str<char32_t>::null();

};

template <>
struct std::hash<aka::String>
{
	size_t operator()(const aka::String& string) const
	{
		return aka::hash::fnv(string.cstr(), string.length());
	}
};