#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <iostream>

#include <Aka/Core/Debug.h>
#include <Aka/Core/Geometry.h>

namespace aka {

template <typename T = char>
class Str final
{
public:
	using Char = T;
	static const size_t invalid = -1;
private:
	static const size_t defaultCapacity = 16;
public:
	Str();
	Str(const std::basic_string<T>& string);
	Str(const Str& string);
	Str(Str&& string);
	Str(const T* string);
	Str(const T* string, size_t length);
	Str(size_t length, T character);
	explicit Str(size_t length);
	Str& operator=(const Str& string);
	Str& operator=(Str&& string);
	Str& operator=(const T* string);
	~Str();

	operator std::basic_string<T>() const;

	T& operator[](size_t index);
	const T& operator[](size_t index) const;

	bool operator==(const Str& string) const;
	bool operator!=(const Str& string) const;
	bool operator<(const Str& string) const;
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

	// Find a character and return its position
	size_t find(T character, size_t offset = 0) const;
	// Find the first occurence of the character and return its position
	size_t findFirst(T character, size_t offset = 0) const;
	// Find the last occurence of the character and return its position
	size_t findLast(T character, size_t offset = 0) const;
	// Find a character and split the string
	std::vector<Str> split(T c) const;
	// Create a new string from start to end of string
	Str substr(size_t start) const;
	// Create a new string from start to end
	Str substr(size_t start, size_t len) const;

private:
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
inline Str<T>::Str(const std::basic_string<T>& string) :
	Str<T>(string.c_str(), string.length())
{
}
template<typename T>
inline Str<T>::Str(const Str<T>& str) :
	Str<T>(str.m_string, str.m_length)
{
}
template<typename T>
inline Str<T>::Str(Str<T>&& string) :
	m_string(nullptr),
	m_length(0),
	m_capacity(0)
{
	std::swap(m_string, string.m_string);
	std::swap(m_length, string.m_length);
	std::swap(m_capacity, string.m_capacity);
}
template<typename T>
inline Str<T>::Str(const T* str) :
	Str<T>(str, Str<T>::length(str))
{
}
template<typename T>
inline Str<T>::Str(const T* str, size_t length) :
	m_string(new T[max(length + 1, defaultCapacity)]),
	m_length(length),
	m_capacity(max(length + 1, defaultCapacity))
{
	Str<T>::copy(m_string, m_capacity, str);
	m_string[length] = '\0';
}
template<typename T>
inline Str<T>::Str(size_t length, T character) :
	m_string(new T[max(length + 1, defaultCapacity)]),
	m_length(length),
	m_capacity(max(length + 1, defaultCapacity))
{
	for (size_t i = 0; i < length; i++)
		m_string[i] = character;
	m_string[length] = '\0';
}
template<typename T>
inline Str<T>::Str(size_t length) :
	m_string(new T[max(length + 1, defaultCapacity)]()),
	m_length(length),
	m_capacity(max(length + 1, defaultCapacity))
{
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
	delete[] m_string;
}
template<typename T>
inline Str<T>::operator std::basic_string<T>() const
{
	return std::basic_string<T>(m_string);
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
	while (m_capacity < size)
		m_capacity *= 2;
	T* buffer = new T[m_capacity];
	Str<T>::copy(buffer, m_capacity, m_string);
	delete[] m_string;
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
	size_t size = 0;
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
	const size_t fmtSize = 256;
	char buffer[fmtSize];
	int error = snprintf(buffer, fmtSize, string, args...);
	AKA_ASSERT(error > 0 && error < fmtSize, "Invalid formatting");
	return String(buffer);
}
template <>
template <typename ...Args>
inline Str<wchar_t> Str<wchar_t>::format(const wchar_t* string, Args ...args)
{
	const size_t fmtSize = 256;
	wchar_t buffer[fmtSize];
	int error = swprintf(buffer, fmtSize, string, args...);
	AKA_ASSERT(error > 0 && error < fmtSize, "Invalid formatting");
	return StringWide(buffer);
}
template <>
template <typename ...Args>
inline Str<char16_t> Str<char16_t>::format(const char16_t* string, Args ...args)
{
	return String16();
}
template <>
template <typename ...Args>
inline Str<char32_t> Str<char32_t>::format(const char32_t* string, Args ...args)
{
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
	for (size_t i = m_length - 1; i >= 1; --i)
		if (character == m_string[i])
			return i;
	return invalid;
}
template <typename T>
inline std::vector<Str<T>> Str<T>::split(T c) const
{
	size_t offset = 0;
	std::vector<Str<T>> strings;
	for (size_t i = 0; i < m_length; ++i)
	{
		if (m_string[i] == c && offset != i)
		{
			strings.push_back(substr(offset, i - offset));
			offset = i + 1;
		}
	}
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