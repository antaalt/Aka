#pragma once

#include <Aka/Core/String.h>

namespace aka {
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
String utf8(const StringWide& wcharString);
// convert utf16 to utf8
String utf8(const String16& utf16String);
// convert utf32 to utf8
String utf8(const String32& utf32String);
// convert utf8 to wchar.
StringWide wide(const String& utf8String);
// convert utf8 to utf16.
String16 utf16(const String& utf8String);
// convert utf8 to utf32.
String32 utf32(const String& utf8String);

}


};