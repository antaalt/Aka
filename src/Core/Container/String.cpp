#include <Aka/Core/Container/String.h>

#include <Aka/Platform/Platform.h>
#include <Aka/Core/Geometry.h>
#include <string.h>

#if defined(AKA_PLATFORM_WINDOWS)
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS
#endif

namespace aka {

template <>
const char* Str<char>::null()
{
	return "";
}
template <>
const wchar_t* Str<wchar_t>::null()
{
	return L"";
}
template <>
const char16_t* Str<char16_t>::null()
{
	return u"";
}
template <>
const char32_t* Str<char32_t>::null()
{
	return U"";
}
template class Str<char>;
template class Str<wchar_t>;
template class Str<char16_t>;
template class Str<char32_t>;

} // namespace aka
