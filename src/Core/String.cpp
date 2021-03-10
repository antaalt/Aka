#include <Aka/Core/String.h>

#include <Aka/Platform/Platform.h>
#include <Aka/Core/Geometry.h>
#include <utf8.h>
#include <algorithm>
#include <string.h>

#if defined(AKA_PLATFORM_WINDOWS)
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS
#endif

namespace aka {

static const size_t defaultCapacity = 16;

String::String() :
	String("", 0)
{
}
String::String(const std::string& string) :
	String(string.c_str(), string.length())
{
}
String::String(const String& str) :
	String(str.m_string, str.m_length)
{
}
String::String(String&& string) :
	m_string(nullptr),
	m_length(0),
	m_capacity(0)
{
	std::swap(m_string, string.m_string);
	std::swap(m_length, string.m_length);
	std::swap(m_capacity, string.m_capacity);
}
String::String(char character) :
	m_string(new char[defaultCapacity]),
	m_length(1),
	m_capacity(defaultCapacity)
{
	m_string[0] = character;
	m_string[1] = '\0';
}
String::String(const char* str) :
	String(str, String::length(str))
{
}
String::String(const char* str, size_t length) :
	m_string(new char[max(length + 1, defaultCapacity)]),
	m_length(length),
	m_capacity(max(length + 1, defaultCapacity))
{
	String::copy(m_string, m_capacity, str);
	m_string[length] = '\0';
}
String::String(size_t length, char character) :
	m_string(new char[max(length + 1, defaultCapacity)]),
	m_length(length),
	m_capacity(max(length + 1, defaultCapacity))
{
	for (size_t i = 0; i < length; i++)
		m_string[i] = character;
	m_string[length] = '\0';
}
String::String(size_t length) :
	m_string(new char[max(length + 1, defaultCapacity)]()),
	m_length(length),
	m_capacity(max(length + 1, defaultCapacity))
{
}
String& String::operator=(const String& str)
{
	resize(str.m_length);
	String::copy(m_string, str.m_capacity, str.m_string);
	return *this;
}
String& String::operator=(String&& str)
{
	std::swap(m_string, str.m_string);
	std::swap(m_length, str.m_length);
	std::swap(m_capacity, str.m_capacity);
	return *this;
}
String& String::operator=(const char* str)
{
	size_t length = String::length(str);
	resize(length);
	String::copy(m_string, length + 1, str);
	return *this;
}
String::~String()
{
	delete[] m_string;
}
String::operator std::string() const
{
	return std::string(m_string);
}
char& String::operator[](size_t index)
{
	ASSERT(index < m_length, "Out of range");
	return m_string[index];
}
const char& String::operator[](size_t index) const
{
	ASSERT(index < m_length, "Out of range");
	return m_string[index];
}
bool String::operator==(const String& str) const
{
	return String::compare(m_string, str.m_string);
}
bool String::operator!=(const String& str) const
{
	return !String::compare(m_string, str.m_string);
}
bool String::operator<(const String& str) const
{
	return std::lexicographical_compare(begin(), end(), str.begin(), str.end());
}
bool String::operator==(const char* str) const
{
	return String::compare(m_string, str);
}
bool String::operator!=(const char* str) const
{
	return !String::compare(m_string, str);
}
String& String::operator+=(const String& str)
{
	return append(str);
}
String& String::operator+=(const char* str)
{
	return append(str);
}
String& String::operator+=(char c)
{
	return append(c);
}
String String::operator+(const String& str) const
{
	String out(*this);
	out.append(str);
	return out;
}
String String::operator+(const char* str) const
{
	String out(*this);
	out.append(str);
	return out;
}
String String::operator+(char c) const
{
	String out(*this);
	out.append(c);
	return out;
}
String& String::append(const String& str)
{
	size_t off = m_length;
	size_t len = str.length();
	resize(off + len);
	String::copy(m_string + off, len + 1, str.cstr());
	return *this;
}
String& String::append(const char* str)
{
	size_t off = m_length;
	size_t len = String::length(str);
	resize(off + len);
	String::copy(m_string + off, len + 1, str);
	return *this;
}
String& String::append(char c)
{
	size_t len = m_length + 1;
	resize(len);
	m_string[len - 1] = c;
	m_string[len] = '\0';
	return *this;
}
size_t String::length() const
{
	return m_length;
}
size_t String::capacity() const
{
	return m_capacity;
}
void String::resize(size_t length)
{
	if (m_length == length)
		return;
	reserve(length + 1);
	m_length = length;
}
void String::reserve(size_t size)
{
	if (size <= m_capacity)
		return;
	while (m_capacity < size)
		m_capacity *= 2;
	char* buffer = new char[m_capacity];
	String::copy(buffer, m_capacity, m_string);
	delete[] m_string;
	m_string = buffer;
}
void String::clear()
{
	*this = String("");
}
bool String::empty() const
{
	return m_length == 0;
}
char* String::cstr()
{
	return m_string;
}
const char* String::cstr() const
{
	return m_string;
}
char String::last() const
{
	return m_string[m_length - 1];
}
char* String::begin()
{
	return m_string;
}
char* String::end()
{
	return m_string + m_length;
}
const char* String::begin() const
{
	return m_string;
}
const char* String::end() const
{
	return m_string + m_length;
}
template <>
const char* String::null<char>()
{
	return "";
}
template <>
const wchar_t* String::null<wchar_t>()
{
	return L"";
}
template <>
const char16_t* String::null<char16_t>()
{
	return u"";
}
template <>
const char32_t* String::null<char32_t>()
{
	return U"";
}
template <>
char* String::copy<char>(char* dst, size_t dstSize, const char* src)
{
	strncpy(dst, src, dstSize);
	dst[dstSize - 1] = '\0'; // Avoid buffer overflow
	return dst;
}
template <>
wchar_t* String::copy<wchar_t>(wchar_t* dst, size_t dstSize, const wchar_t* src)
{
	wcsncpy(dst, src, dstSize);
	dst[dstSize - 1] = L'\0'; // Avoid buffer overflow
	return dst;
}
template <>
char16_t* String::copy<char16_t>(char16_t* dst, size_t dstSize, const char16_t* src)
{
	size_t size = 0;
	char16_t* tmp = dst;
	while ((*tmp++ = *src++) != 0 && size++ < dstSize);
	dst[dstSize - 1] = L'\0'; // Avoid buffer overflow
	return dst;
}
template <>
char32_t* String::copy<char32_t>(char32_t* dst, size_t dstSize, const char32_t* src)
{
	size_t size = 0;
	char32_t* tmp = dst;
	while ((*tmp++ = *src++) != 0 && size++ < dstSize);
	dst[dstSize - 1] = L'\0'; // Avoid buffer overflow
	return dst;
}
template <>
size_t String::length<char>(const char* string)
{
	return strlen(string);
}
template <>
size_t String::length<wchar_t>(const wchar_t* string)
{
	return wcslen(string);
}
template <>
size_t String::length<char16_t>(const char16_t* string)
{
	if (string == nullptr) return 0;
	size_t size = 0;
	const char16_t* tmp = string;
	for (; (*tmp) != 0; ++tmp);
	return tmp - string;
}
template <>
size_t String::length<char32_t>(const char32_t* string)
{
	if (string == nullptr) return 0;
	size_t size = 0;
	const char32_t* tmp = string;
	for (; (*tmp) != 0; ++tmp);
	return tmp - string;
}
template <>
bool String::compare<char>(const char* lhs, const char* rhs)
{
	return strcmp(lhs, rhs) == 0;
}
template <>
bool String::compare<wchar_t>(const wchar_t* lhs, const wchar_t* rhs)
{
	return wcscmp(lhs, rhs) == 0;
}
template <>
bool String::compare<char16_t>(const char16_t* lhs, const char16_t* rhs)
{
	const char16_t* tmpl = lhs;
	const char16_t* tmpr = rhs;
	do
	{
		if (*tmpl++ != *tmpr++)
			return false;
	} while (*tmpl != 0 && *tmpr != 0);
	return true;
}
template <>
bool String::compare<char32_t>(const char32_t* lhs, const char32_t* rhs)
{
	const char32_t* tmpl = lhs;
	const char32_t* tmpr = rhs;
	do
	{
		if (*tmpl++ != *tmpr++)
			return false;
	} while (*tmpl != 0 && *tmpr != 0);
	return true;
}
char String::lowercase(char c)
{
	if (c <= 'z' && c >= 'a')
		return c; // already lowercase
	if (c > 'Z' || c < 'A')
		return c; // not in range
	return 'a' + (c - 'A');
}
char String::uppercase(char c)
{
	if (c <= 'Z' && c >= 'A')
		return c; // already uppercase
	if (c > 'z' || c < 'a')
		return c; // not in range
	return 'A' + (c - 'a');
}
String& String::lowercase(String& string)
{
	for (char& c : string)
		c = lowercase(c);
	return string;
}
String& String::uppercase(String& string)
{
	for (char& c : string)
		c = uppercase(c);
	return string;
}
void String::trim()
{
	throw std::runtime_error("Not implemented");
}
size_t String::find(char character, size_t offset) const
{
	return findFirst(character, offset);
}
size_t String::findFirst(char character, size_t offset) const
{
	for (size_t i = offset; i < m_length; ++i)
		if (character == m_string[i])
			return i;
	return invalid;
}
size_t String::findLast(char character, size_t offset) const
{
	for (size_t i = m_length - 1; i >= 1; --i)
		if (character == m_string[i])
			return i;
	return invalid;
}
std::vector<String> String::split(char c) const
{
	size_t offset = 0;
	std::vector<String> strings;
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
String String::substr(size_t start) const
{
	return String(m_string + start, m_length - start);
}
String String::substr(size_t start, size_t len) const
{
	return String(m_string + start, len);
}

std::ostream& operator<<(std::ostream& os, const String& str)
{
	os << str.cstr();
	return os;
}
std::ostream& operator<<(std::ostream& os, const Str<char>& str)
{
	os << str.cstr();
	return os;
}
std::ostream& operator<<(std::ostream& os, const Str<wchar_t>& str)
{
	os << str.cstr();
	return os;
}
std::ostream& operator<<(std::ostream& os, const Str<char16_t>& str)
{
	os << str.cstr();
	return os;
}
std::ostream& operator<<(std::ostream& os, const Str<char32_t>& str)
{
	os << str.cstr();
	return os;
}

namespace encoding {

template <class T>
class StrBackInsertIterator {
public:
	using iterator_category = std::output_iterator_tag;
	using value_type = void;
	using pointer = void;
	using reference = void;
	using container_type = Str<T>;
	explicit StrBackInsertIterator(Str<T>& c) noexcept : container(&c) {}

	StrBackInsertIterator& operator=(const T& val) {
		container->append(val);
		return *this;
	}

	StrBackInsertIterator& operator=(T&& val) {
		container->append(std::move(val));
		return *this;
	}
	StrBackInsertIterator& operator*() noexcept { return *this; }
	StrBackInsertIterator& operator++() noexcept { return *this; }
	StrBackInsertIterator operator++(int) noexcept { return *this; }

protected:
	Str<T>* container = nullptr;
};

CodePoint next(const char*& start, const char* end)
{
	return utf8::next(start, end);
}
size_t length(const String& utf8String)
{
	return utf8::distance(utf8String.begin(), utf8String.end());
}
String ascii(const String& utf8String)
{
	String asciiString;
	// Strip all special utf8 characters
	for (const char& c : utf8String)
		if ((uint8_t)c < 128)
			asciiString += c;
	return asciiString;
}
String utf8(const String& asciiString)
{
	return asciiString;
}
String utf8(const StrWide& wcharString)
{
	Str str;
	if constexpr (sizeof(wchar_t) == 2)
		utf8::utf16to8(wcharString.begin(), wcharString.end(), StrBackInsertIterator(str));
	else if constexpr (sizeof(wchar_t) == 4)
		utf8::utf32to8(wcharString.begin(), wcharString.end(), StrBackInsertIterator(str));
	return String(str.cstr());
}
String utf8(const Str16& utf16String)
{
	Str str;
	utf8::utf16to8(utf16String.begin(), utf16String.end(), StrBackInsertIterator(str));
	return String(str.cstr());
}
String utf8(const Str32& utf32String)
{
	Str str;
	utf8::utf32to8(utf32String.begin(), utf32String.end(), StrBackInsertIterator(str));
	return String(str.cstr());
}
StrWide wide(const String& utf8String)
{
	StrWide wstr;
	if constexpr (sizeof(wchar_t) == 2)
		utf8::utf8to16(utf8String.begin(), utf8String.end(), StrBackInsertIterator(wstr));
	else if constexpr (sizeof(wchar_t) == 4)
		utf8::utf8to32(utf8String.begin(), utf8String.end(), StrBackInsertIterator(wstr));
	return wstr;
}
Str16 utf16(const String& utf8String)
{
	Str16 u16Str;
	utf8::utf8to16(utf8String.begin(), utf8String.end(), StrBackInsertIterator(u16Str));
	return u16Str;
}
Str32 utf32(const String& utf8String)
{
	Str32 u32Str;
	utf8::utf8to32(utf8String.begin(), utf8String.end(), StrBackInsertIterator(u32Str));
	return u32Str;
}
} // namespace encoding
} // namespace aka
