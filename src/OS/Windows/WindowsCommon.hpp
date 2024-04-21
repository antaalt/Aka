#pragma once

#if defined(AKA_PLATFORM_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <shlwapi.h>

#include <filesystem>
#include <iostream>

namespace aka {

inline aka::StringWide Utf8ToWchar(const char* str)
{
	int wstr_size = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
	aka::StringWide wstr(wstr_size - 1);
	int ret = MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr.cstr(), wstr_size);
	AKA_ASSERT(ret != 0, "Failed to convert string"); // GetLastError()
	return wstr;
}

inline aka::String WcharToUtf8(const wchar_t* wstr)
{
	int str_size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, NULL, NULL);
	aka::String str(str_size - 1);
	int ret = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str.cstr(), str_size, NULL, NULL);
	AKA_ASSERT(ret != 0, "Failed to convert string"); // GetLastError()
	return str;
}

inline void BackwardToForwardSlash(String& string)
{
	for (char& c : string)
	{
		if (c == '\\')
			c = '/';
	}
}
inline void RemoveNeighborDuplicateSlash(String& string)
{
	uint32_t offset = 0;
	for (uint32_t i = 0; i < string.size() - offset; i++)
	{
		string[i] = string[i + offset];
		uint32_t count = 1;
		while (string[i + offset] == '/' && string[i + offset + count] == '/')
		{
			count++;
		}
		offset += count - 1;
	}
	string[string.size() - offset] = '\0';
	string.resize(string.size() - offset);
}

inline String GetLastErrorAsString()
{
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
	{
		return String("No errors.");
	}

	LPWSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	StringWide message(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	return WcharToUtf8(message.cstr());
}


};

#endif