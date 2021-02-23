#include <Aka/Platform/PlatformBackend.h>
#include <Aka/Platform/InputBackend.h>
#include <Aka/Platform/Platform.h>
#include <Aka/OS/FileSystem.h>
#include <Aka/OS/Logger.h>

#if defined(AKA_PLATFORM_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <fstream>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

namespace aka {

std::wstring Utf8ToWchar(const std::string& str) {
	int wstr_size = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
	std::wstring wstr(wstr_size, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &wstr[0], (int)wstr.size());
	return wstr;
}

std::string WcharToUtf8(const std::wstring& wstr) {
	int str_size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, NULL, NULL);
	std::string str(str_size, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &str[0], (int)str.size(), NULL, NULL);
	return str;
}

const WORD terminalColors[20] = {
	0, // ForgeroundBlack
	FOREGROUND_RED, // ForegroundRed
	FOREGROUND_GREEN, // ForegroundGreen
	FOREGROUND_RED | FOREGROUND_GREEN, // ForegroundYellow
	FOREGROUND_BLUE, // ForegroundBlue
	FOREGROUND_RED | FOREGROUND_BLUE, // ForegroundMagenta
	FOREGROUND_GREEN | FOREGROUND_BLUE, // ForegroundCyan
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, // ForegroundWhite
	FOREGROUND_INTENSITY, // ForgeroundBrightBlack
	FOREGROUND_RED | FOREGROUND_INTENSITY, // ForegroundBrightRed
	FOREGROUND_GREEN | FOREGROUND_INTENSITY, // ForegroundBrightGreen
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, // ForegroundBrightYellow
	FOREGROUND_BLUE | FOREGROUND_INTENSITY, // ForegroundBrightBlue
	FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY, // ForegroundBrightMagenta
	FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, // ForegroundBrightCyan
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, // ForegroundBrightWhite
};

std::ostream& operator<<(std::ostream& os, Logger::Color color)
{
	if (color == Logger::Color::ForegroundNone)
		return os;
	HANDLE hdl = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleTextAttribute(hdl, terminalColors[(unsigned int)color]);
	return os;
}

void PlatformBackend::errorDialog(const std::string& message)
{
	std::wstring msg = Utf8ToWchar(message);
	int msgBoxID = MessageBox(
		getWindowsWindowHandle(),
		msg.data(),
		L"Error",
		MB_ICONERROR | MB_OK | MB_DEFBUTTON1
	);
	switch (msgBoxID)
	{
	case IDOK:
		// TODO: add code
		break;
	}
}

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


std::vector<uint8_t> BinaryFile::load(const Path& path)
{
	std::wstring wstr = Utf8ToWchar(path.str());
	std::basic_ifstream<uint8_t> ifs(wstr, std::ios::binary);
	if (!ifs)
	{
		Logger::error("Could not load binary file ", path);
		return std::vector<uint8_t>();
	}
	return std::vector<uint8_t>((std::istreambuf_iterator<uint8_t>(ifs)), (std::istreambuf_iterator<uint8_t>()));
}

void BinaryFile::write(const Path& path, const std::vector<uint8_t>& bytes)
{
	write(path, bytes.data(), bytes.size());
}

void BinaryFile::write(const Path& path, const uint8_t* bytes, size_t size)
{
	std::wstring wstr = Utf8ToWchar(path.str());
	std::basic_ofstream<uint8_t> ofs(wstr, std::ios::binary);
	if (!ofs)
	{
		Logger::error("Could not write binary file ", path);
		return;
	}
	ofs.write(bytes, size);
}

std::string TextFile::load(const Path& path)
{
	std::wstring wstr = Utf8ToWchar(path.str());
	std::ifstream ifs(wstr);
	if (!ifs)
	{
		Logger::error("Could not load text file ", path);
		return std::string();
	}
	return std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
}

void TextFile::write(const Path& path, const std::string& str)
{
	write(path, str.c_str());
}

void TextFile::write(const Path& path, const char* str)
{
	std::wstring wstr = Utf8ToWchar(path.str());
	std::ofstream ofs(wstr);
	if (!ofs)
	{
		Logger::error("Could not write text file ", path);
		return;
	}
	ofs << str;
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

};
#endif
