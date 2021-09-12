#pragma once

#include <Aka/Platform/Platform.h>
#include <Aka/Core/Container/String.h>

#if defined(AKA_PLATFORM_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <shlwapi.h>

inline aka::StringWide Utf8ToWchar(const char* str)
{
	int wstr_size = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
	aka::StringWide wstr(wstr_size - 1);
	MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr.cstr(), (int)wstr.length());
	return wstr;
}

inline aka::String WcharToUtf8(const wchar_t* wstr)
{
	int str_size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, NULL, NULL);
	aka::String str(str_size - 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str.cstr(), (int)str.length(), NULL, NULL);
	return str;
}

#endif