#include <Aka/Core/Encoding.h>

#include <utf8.h>
#include <algorithm>

namespace aka {
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
String utf8(const StringWide& wcharString)
{
	Str str;
	if constexpr (sizeof(wchar_t) == 2)
		utf8::utf16to8(wcharString.begin(), wcharString.end(), StrBackInsertIterator(str));
	else if constexpr (sizeof(wchar_t) == 4)
		utf8::utf32to8(wcharString.begin(), wcharString.end(), StrBackInsertIterator(str));
	return String(str.cstr());
}
String utf8(const String16& utf16String)
{
	Str str;
	utf8::utf16to8(utf16String.begin(), utf16String.end(), StrBackInsertIterator(str));
	return String(str.cstr());
}
String utf8(const String32& utf32String)
{
	Str str;
	utf8::utf32to8(utf32String.begin(), utf32String.end(), StrBackInsertIterator(str));
	return String(str.cstr());
}
StringWide wide(const String& utf8String)
{
	StringWide wstr;
	if constexpr (sizeof(wchar_t) == 2)
		utf8::utf8to16(utf8String.begin(), utf8String.end(), StrBackInsertIterator(wstr));
	else if constexpr (sizeof(wchar_t) == 4)
		utf8::utf8to32(utf8String.begin(), utf8String.end(), StrBackInsertIterator(wstr));
	return wstr;
}
String16 utf16(const String& utf8String)
{
	String16 u16Str;
	utf8::utf8to16(utf8String.begin(), utf8String.end(), StrBackInsertIterator(u16Str));
	return u16Str;
}

String32 utf32(const String& utf8String)
{
	String32 u32Str;
	utf8::utf8to32(utf8String.begin(), utf8String.end(), StrBackInsertIterator(u32Str));
	return u32Str;
}

}; // namespace encoding
}; // namespace aka