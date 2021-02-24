#pragma once

#include <Aka/Platform/Platform.h>

#if defined(AKA_PLATFORM_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <shlwapi.h>

inline std::wstring Utf8ToWchar(const std::string& str)
{
	int wstr_size = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
	std::wstring wstr(wstr_size, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &wstr[0], (int)wstr.size());
	return wstr;
}

inline std::string WcharToUtf8(const std::wstring& wstr)
{
	int str_size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, NULL, NULL);
	std::string str(str_size, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &str[0], (int)str.size(), NULL, NULL);
	return str;
}

#endif