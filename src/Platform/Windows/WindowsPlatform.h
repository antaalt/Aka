#pragma once

#include <Aka/Platform/Platform.h>
#include <Aka/Core/String.h>

#if defined(AKA_PLATFORM_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <shlwapi.h>

inline aka::StringWide Utf8ToWchar(const aka::String& str)
{
	int wstr_size = MultiByteToWideChar(CP_UTF8, 0, str.cstr(), (int)str.length(), nullptr, 0);
	aka::StringWide wstr(wstr_size);
	MultiByteToWideChar(CP_UTF8, 0, str.cstr(), (int)str.length(), wstr.cstr(), (int)wstr.length());
	return wstr;
}

inline aka::String WcharToUtf8(const aka::StringWide& wstr)
{
	int str_size = WideCharToMultiByte(CP_UTF8, 0, wstr.cstr(), (int)wstr.length(), nullptr, 0, NULL, NULL);
	aka::String str(str_size, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.cstr(), (int)wstr.length(), str.cstr(), (int)str.length(), NULL, NULL);
	return str;
}

#endif