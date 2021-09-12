#include <Aka/OS/FileSystem.h>
#include <Aka/Platform/Platform.h>
#include <Aka/OS/Logger.h>
#include <Aka/OS/Stream/FileStream.h>

#include "WindowsPlatform.h"

#if defined(AKA_PLATFORM_WINDOWS)

namespace aka {

bool Directory::exist(const Path& path)
{
	StringWide wstr = Utf8ToWchar(path.str());
	DWORD ftyp = GetFileAttributes(wstr.cstr());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false; // Incorrect path
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true; // Directory
	return false;
}

bool Directory::create(const Path& path)
{
	size_t pos = 0;
	const String& str = path.str();
	do
	{
		pos = str.findFirst('/', pos + 1);
		if (pos == str.length())
			return true;
		String p = str.substr(0, pos);
		if (p == "." || p == ".." || p == "/" || p == "\\")
			continue;
		StringWide wstr = Utf8ToWchar(p);
		if (!CreateDirectory(wstr.cstr(), NULL))
		{
			DWORD error = GetLastError();
			if (ERROR_ALREADY_EXISTS == error)
				continue;
			else if (ERROR_SUCCESS != error)
				return false;
		}
	} while (pos != String::invalid);
	return true;
}

bool Directory::remove(const Path& path, bool recursive)
{
	if (recursive)
	{
		StringWide wstr = Utf8ToWchar(path.str());
		WIN32_FIND_DATA data;
		wchar_t cwd[512];
		GetCurrentDirectory(512, cwd);
		SetCurrentDirectory(wstr.cstr());
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
	StringWide str = Utf8ToWchar(path.str());
	return RemoveDirectory(str.cstr()) == TRUE;
}

bool File::exist(const Path& path)
{
	StringWide str = Utf8ToWchar(path.str());
	DWORD ftyp = GetFileAttributes(str.cstr());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!
	return (!(ftyp & FILE_ATTRIBUTE_DIRECTORY));
}
bool File::create(const Path& path)
{
	StringWide wstr = Utf8ToWchar(path.str());
	HANDLE h = CreateFile(wstr.cstr(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (h)
	{
		CloseHandle(h);
		return true;
	}
	else
	{
		return false; // GetLastError()
	}
}
bool File::remove(const Path& path)
{
	StringWide str = Utf8ToWchar(path.str());
	return DeleteFile(str.cstr()) == TRUE;
}

String File::extension(const Path& path)
{
	StringWide wstr = Utf8ToWchar(path.str());
	LPWSTR extension = PathFindExtension(wstr.cstr());
	String out = WcharToUtf8(extension);
	if (out.length() < 1)
		return String();
	return out.substr(1, out.length() - 1);
}

String File::name(const Path& path)
{
	StringWide str = Utf8ToWchar(path.str());
	LPWSTR FileName = PathFindFileName(str.cstr());
	return WcharToUtf8(FileName);
}
size_t File::size(const Path& path)
{
	StringWide wstr = Utf8ToWchar(path.str());
	HANDLE h = CreateFile(wstr.cstr(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (h)
	{
		LARGE_INTEGER size;
		if (!GetFileSizeEx(h, &size))
		{
			CloseHandle(h);
			return 0;
		}
		CloseHandle(h);
		return size.QuadPart;
	}
	else
	{
		return 0; // GetLastError()
	}
}

std::vector<Path> Path::enumerate(const Path& path)
{
	const wchar_t separator = '/';
	StringWide wstr = Utf8ToWchar(path.str());
	WIN32_FIND_DATA data;
	StringWide searchString;
	if (wstr.last() == separator)
		searchString = wstr.append(L'*');
	else
		searchString = wstr.append(separator).append(L'*');
	HANDLE hFind = FindFirstFile(searchString.cstr(), &data);
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
				// Directory
				String str = WcharToUtf8(data.cFileName);
				paths.push_back(str + '/');
			}
			else
			{
				// File
				String str = WcharToUtf8(data.cFileName);
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
	StringWide wstr = Utf8ToWchar(path.str());
	if (PathCanonicalize(canonicalizedPath, wstr.cstr()) == TRUE)
	{
		String str = WcharToUtf8(canonicalizedPath);
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
	String str = WcharToUtf8(path);
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
	String str = WcharToUtf8(path);
	for (char& c : str)
		if (c == '\\')
			c = '/';
	return Path(str + '/');
}

const wchar_t* fileMode(FileMode mode, FileType type)
{
	switch (type)
	{
	case aka::FileType::Binary:
		if (mode == FileMode::Read)
			return L"rb";
		else if (mode == FileMode::Write)
			return L"wb";
		else if (mode == FileMode::ReadWrite)
			return L"rwb";
		break;
	case aka::FileType::String:
		if (mode == FileMode::Read)
			return L"r";
		else if (mode == FileMode::Write)
			return L"w";
		else if (mode == FileMode::ReadWrite)
			return L"rw";
		break;
	default:
		break;
	}
	return L"";
}

FILE* fopen(const Path& path, FileMode mode, FileType type)
{
	StringWide wstr = Utf8ToWchar(path.str());
	FILE* File = nullptr;
	errno_t err = _wfopen_s(&File, wstr.cstr(), fileMode(mode, type));
	if (err == 0)
		return File;
	return nullptr;
}

};

#endif