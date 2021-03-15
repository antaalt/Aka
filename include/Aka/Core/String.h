#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <iostream>

#include <Aka/Core/Debug.h>

namespace aka {

class String final
{
public:
	using Char = char;
	static const size_t invalid = -1;
public:
	String();
	String(const std::string& string);
	String(const String& string);
	String(String&& string);
	String(const char* string);
	String(const char* string, size_t length);
	String(size_t length, char character);
	explicit String(size_t length);
	String& operator=(const String& string);
	String& operator=(String&& string);
	String& operator=(const char* string);
	~String();

	operator std::string() const;

	char& operator[](size_t index);
	const char& operator[](size_t index) const;

	bool operator==(const String& string) const;
	bool operator!=(const String& string) const;
	bool operator<(const String& string) const;
	bool operator==(const char* string) const;
	bool operator!=(const char* string) const;

	String& operator+=(const String& string);
	String& operator+=(const char* string);
	String& operator+=(char character);
	String operator+(const String& string) const;
	String operator+(const char* string) const;
	String operator+(char character) const;

	String& append(const String& string);
	String& append(const char* string);
	String& append(char character);

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
	char* cstr();
	// Pointer to the string
	const char* cstr() const;
	// Last character of string
	char last() const;
	// Pointer to beginning of string
	char* begin();
	// Pointer to ending of string
	char* end();
	// Pointer to beginning of string
	const char* begin() const;
	// Pointer to ending of string
	const char* end() const;
	
	// Get a null raw string
	template <typename T>
	static const T* null();
	// Copy a raw string in another one
	template <typename T>
	static T* copy(T* dst, size_t dstSize, const T* src);
	// Get the length of a raw string
	template <typename T>
	static size_t length(const T* string);
	// Compare a raw string with another one
	template <typename T>
	static bool compare(const T* lhs, const T* rhs);
	// Format a raw string
	template <typename ...Args>
	static String format(const char* string, Args ...args);

	// Lowercase a single character
	static char lowercase(char c);
	// Uppercase a single character
	static char uppercase(char c);

	// Lowercase a string
	static String& lowercase(String& string);
	// Uppercase a string
	static String& uppercase(String& string);

	// charrim whitespace at beginning and end
	void trim();
	// Find a character and return its position
	size_t find(char character, size_t offset = 0) const;
	// Find the first occurence of the character and return its position
	size_t findFirst(char character, size_t offset = 0) const;
	// Find the last occurence of the character and return its position
	size_t findLast(char character, size_t offset = 0) const;
	// Find a character and split the string
	std::vector<String> split(char c) const;
	// Create a new string from start to end of string
	String substr(size_t start) const;
	// Create a new string from start to end
	String substr(size_t start, size_t len) const;

private:
	char* m_string;
	size_t m_length;
	size_t m_capacity;
};

// Basic wrapper that support wide string
template <typename T = char>
class Str final
{
public:
	using Type = T;
public:
	Str() : Str(String::null<T>(), 0) {}
	Str(const Str& string) : Str(string.m_string, string.m_length) {}
	Str(const T* string) : Str(string, String::length<T>(string)) {}
	Str(size_t length) : Str(String::null<T>(), length) {}
	Str(const T* string, size_t length) : 
		m_string(new T[length + 1]), m_length(length), m_capacity(length + 1) { String::copy(m_string, length + 1, string); }
	~Str() { delete[] m_string; }
	void resize(size_t length)
	{
		if (length < m_capacity)
			m_length = length;
		else
		{
			m_length = length;
			while (m_capacity <= m_length)
				m_capacity *= 2;
			T* buffer = new T[m_capacity];
			String::copy<T>(buffer, m_capacity, m_string);
			delete[] m_string;
			m_string = buffer;
		}
	}
	Str<T>& append(T c)
	{
		resize(m_length + 1);
		m_string[m_length - 1] = c;
		m_string[m_length] = String::null<T>()[0];
		return *this;
	}
	T& operator[](size_t index) { return m_string[index]; }
	const T& operator[](size_t index) const { return m_string[index]; }
	T* cstr() { return m_string; }
	const T* cstr() const { return m_string; }
	T last() const { return m_string[m_length - 1]; }
	size_t length() const { return m_length; }
	T* begin() { return m_string; }
	T* end() { return m_string + m_length; }
	const T* begin() const { return m_string; }
	const T* end() const { return m_string + m_length; }
private:
	T* m_string;
	size_t m_length;
	size_t m_capacity;
};

using StrWide = Str<wchar_t>;
using Str16 = Str<char16_t>;
using Str32 = Str<char32_t>;

inline String operator+(char c, const String& str) { return String().append(c) + str; }
inline String operator+(const char* s, const String& str) { return String(s) + str; }

template <> const char* String::null<char>();
template <> const wchar_t* String::null<wchar_t>();
template <> const char16_t* String::null<char16_t>();
template <> const char32_t* String::null<char32_t>();
template <> char* String::copy<char>(char* dst, size_t dstSize, const char* src);
template <> wchar_t* String::copy<wchar_t>(wchar_t* dst, size_t dstSize, const wchar_t* src);
template <> char16_t* String::copy<char16_t>(char16_t* dst, size_t dstSize, const char16_t* src);
template <> char32_t* String::copy<char32_t>(char32_t* dst, size_t dstSize, const char32_t* src);
template <> size_t String::length<char>(const char* string);
template <> size_t String::length<wchar_t>(const wchar_t* string);
template <> size_t String::length<char16_t>(const char16_t* string);
template <> size_t String::length<char32_t>(const char32_t* string);
template <> bool String::compare<char>(const char* lhs, const char* rhs);
template <> bool String::compare<wchar_t>(const wchar_t* lhs, const wchar_t* rhs);
template <> bool String::compare<char16_t>(const char16_t* lhs, const char16_t* rhs);
template <> bool String::compare<char32_t>(const char32_t* lhs, const char32_t* rhs);
template <typename ...Args>
static String String::format(const char* string, Args ...args)
{
	const size_t fmtSize = 256;
	char buffer[fmtSize];
	int error = snprintf(buffer, fmtSize, string, args...);
	AKA_ASSERT(error > 0 && error < fmtSize, "Invalid formatting");
	return String(buffer);
}

std::ostream& operator<<(std::ostream& os, const String& str);
std::ostream& operator<<(std::ostream& os, const Str<>& str);
std::ostream& operator<<(std::ostream& os, const StrWide& str);
std::ostream& operator<<(std::ostream& os, const Str16& str);
std::ostream& operator<<(std::ostream& os, const Str32& str);

namespace encoding {

using CodePoint = uint32_t;

// Read utf32 character from utf8 string, character by character.
CodePoint next(const char*& start, const char* end);

// Get the correct length of an utf8 encoded string.
size_t length(const String& utf8String);

// convert uf8 to ascii.
String ascii(const String& utf8String);
// convert ascii to utf8.
String utf8(const String& asciiString);
// convert wchar to utf8
String utf8(const StrWide& wcharString);
// convert utf16 to utf8
String utf8(const Str16& utf16String);
// convert utf32 to utf8
String utf8(const Str32& utf32String);
// convert utf8 to wchar.
StrWide wide(const String& utf8String);
// convert utf8 to utf16.
Str16 utf16(const String& utf8String);
// convert utf8 to utf32.
Str32 utf32(const String& utf8String);

}


};