#include <Aka/OS/FileSystem.h>
#include <Aka/Platform/Platform.h>
#include <Aka/OS/Logger.h>

#include "WindowsPlatform.h"

#if defined(AKA_PLATFORM_WINDOWS)

namespace aka {

bool directory::exist(const Path& path)
{
	std::wstring wstr = Utf8ToWchar(path.str());
	DWORD ftyp = GetFileAttributes(wstr.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false; // Incorrect path
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true; // Directory
	return false;
}

bool directory::create(const Path& path)
{
	size_t pos = 0;
	const std::string& str = path.str();
	do
	{
		pos = str.find_first_of("\\/", pos + 1);
		if (pos == str.length())
			return true;
		std::string p = str.substr(0, pos);
		if (p == "." || p == ".." || p == "/" || p == "\\")
			continue;
		std::wstring wstr = Utf8ToWchar(p);
		if (!CreateDirectory(wstr.c_str(), NULL))
		{
			DWORD error = GetLastError();
			if (ERROR_ALREADY_EXISTS == error)
				continue;
			else if (ERROR_SUCCESS != error)
				return false;
		}
	} while (pos != std::string::npos);
	return true;
}

bool directory::remove(const Path& path, bool recursive)
{
	if (recursive)
	{
		std::wstring wstr = Utf8ToWchar(path.str());
		WIN32_FIND_DATA data;
		wchar_t cwd[512];
		GetCurrentDirectory(512, cwd);
		SetCurrentDirectory(wstr.c_str());
		HANDLE hFind = FindFirstFile(L"*", &data);
		do {
			if (data.dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY)
				DeleteFile(data.cFileName);
			else
			{
				Path p = WcharToUtf8(data.cFileName);
				remove(p, true);
			}
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
		SetCurrentDirectory(cwd);
	}
	std::wstring str = Utf8ToWchar(path.str());
	return RemoveDirectory(str.c_str()) == TRUE;
}

bool file::exist(const Path& path)
{
	std::wstring str = Utf8ToWchar(path.str());
	DWORD ftyp = GetFileAttributes(str.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!
	return (!(ftyp & FILE_ATTRIBUTE_DIRECTORY));
}
bool file::create(const Path& path)
{
	std::ofstream file(path.str());
	return file.is_open();
}
bool file::remove(const Path& path)
{
	std::wstring str = Utf8ToWchar(path.str());
	return DeleteFile(str.c_str()) == TRUE;
}

std::string file::extension(const Path& path)
{
	std::wstring wstr = Utf8ToWchar(path.str());
	LPWSTR extension = PathFindExtension(wstr.c_str());
	std::string out = WcharToUtf8(extension);
	if (out.size() < 1)
		return std::string();
	return out.substr(1, out.size() - 1);
}

std::string file::name(const Path& path)
{
	std::wstring str = Utf8ToWchar(path.str());
	LPWSTR fileName = PathFindFileName(str.c_str());
	return WcharToUtf8(fileName);
}

std::vector<Path> Path::enumerate(const Path& path)
{
	const wchar_t separator = '/';
	std::wstring str = Utf8ToWchar(path.str());
	WIN32_FIND_DATA data;
	std::wstring searchString;
	if (str.back() == separator)
		searchString = str + L'*';
	else
		searchString = str + separator + L'*';
	HANDLE hFind = FindFirstFile(searchString.c_str(), &data);
	std::vector<Path> paths;
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (data.dwFileAttributes == INVALID_FILE_ATTRIBUTES)
				continue;
			else if (wcscmp(data.cFileName, L"..") == 0)
				continue;
			else if (wcscmp(data.cFileName, L".") == 0)
				continue;
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// directory
				std::string str = WcharToUtf8(data.cFileName);
				paths.push_back(str + '/');
			}
			else
			{
				// file
				std::string str = WcharToUtf8(data.cFileName);
				paths.push_back(str);
				// size :((unsigned long long)data.nFileSizeHigh * ((unsigned long long)MAXDWORD + 1ULL)) + (unsigned long long)data.nFileSizeLow;
			}
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
	return paths;
}

Path Path::normalize(const Path& path)
{
	WCHAR canonicalizedPath[MAX_PATH];
	std::wstring wstr = Utf8ToWchar(path.str());
	if (PathCanonicalize(canonicalizedPath, wstr.c_str()) == TRUE)
	{
		std::string str = WcharToUtf8(canonicalizedPath);
		for (char& c : str)
			if (c == '\\')
				c = '/';
		return str;
	}
	return path;
}

Path Path::executable()
{
	WCHAR path[MAX_PATH]{};
	if (GetModuleFileName(NULL, path, MAX_PATH) == 0)
		return Path();
	std::string str = WcharToUtf8(path);
	for (char& c : str)
		if (c == '\\')
			c = '/';
	return Path(str);
}

Path Path::cwd()
{
	WCHAR path[MAX_PATH] = { 0 };
	if (GetCurrentDirectory(MAX_PATH, path) == 0)
		return Path();
	std::string str = WcharToUtf8(path);
	for (char& c : str)
		if (c == '\\')
			c = '/';
	return Path(str + '/');
}

const wchar_t* fileMode(FileMode mode)
{
	switch (mode)
	{
	case FileMode::ReadOnly:
		return L"rb";
	case FileMode::WriteOnly:
		return L"wb";
	default:
	case FileMode::ReadWrite:
		return L"rwb";
	}
}

FILE* fopen(const Path& path, FileMode mode)
{
	std::wstring wstr = Utf8ToWchar(path.str());
	return _wfopen(wstr.c_str(), fileMode(mode));
}

};

#endif