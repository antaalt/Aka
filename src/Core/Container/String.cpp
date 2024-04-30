#include <Aka/Core/Container/String.h>

#include <Aka/Platform/Platform.h>
#include <Aka/Core/Geometry.h>
#include <string.h>

#if defined(AKA_PLATFORM_WINDOWS)
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS
#endif

namespace aka {

template class Str<char>;
template class Str<wchar_t>;
template class Str<char16_t>;
template class Str<char32_t>;

} // namespace aka
