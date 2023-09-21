#include <Aka/OS/OS.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Core/Application.h>

#if defined(AKA_PLATFORM_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <shlwapi.h>

#include <filesystem>
#include <iostream>

#pragma comment(lib, "shlwapi.lib")

namespace aka {

const WORD terminalColors[20] = {
	0, // ForegroundBlack
	FOREGROUND_RED, // ForegroundRed
	FOREGROUND_GREEN, // ForegroundGreen
	FOREGROUND_RED | FOREGROUND_GREEN, // ForegroundYellow
	FOREGROUND_BLUE, // ForegroundBlue
	FOREGROUND_RED | FOREGROUND_BLUE, // ForegroundMagenta
	FOREGROUND_GREEN | FOREGROUND_BLUE, // ForegroundCyan
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, // ForegroundWhite
	FOREGROUND_INTENSITY, // ForegroundBrightBlack
	FOREGROUND_RED | FOREGROUND_INTENSITY, // ForegroundBrightRed
	FOREGROUND_GREEN | FOREGROUND_INTENSITY, // ForegroundBrightGreen
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, // ForegroundBrightYellow
	FOREGROUND_BLUE | FOREGROUND_INTENSITY, // ForegroundBrightBlue
	FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY, // ForegroundBrightMagenta
	FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, // ForegroundBrightCyan
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, // ForegroundBrightWhite
};

aka::StringWide Utf8ToWchar(const char* str)
{
	int wstr_size = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
	aka::StringWide wstr(wstr_size - 1);
	int ret = MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr.cstr(), wstr_size);
	AKA_ASSERT(ret != 0, "Failed to convert string"); // GetLastError()
	return wstr;
}

aka::String WcharToUtf8(const wchar_t* wstr)
{
	int str_size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, NULL, NULL);
	aka::String str(str_size - 1);
	int ret = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str.cstr(), str_size, NULL, NULL);
	AKA_ASSERT(ret != 0, "Failed to convert string"); // GetLastError()
	return str;
}

String GetLastErrorAsString()
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

std::ostream& operator<<(std::ostream& os, Logger::Color color)
{
	if (color == Logger::Color::ForegroundNone)
		return os;
	HANDLE hdl = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleTextAttribute(hdl, terminalColors[(unsigned int)color]);
	return os;
}

bool OS::Directory::exist(const Path& path)
{
	StringWide wstr = Utf8ToWchar(path.cstr());
	DWORD ftyp = GetFileAttributes(wstr.cstr());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false; // Incorrect path
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true; // Directory
	return false;
}

bool OS::Directory::create(const Path& path)
{
	size_t pos = 0;
	String str = path.cstr();
	do
	{
		pos = str.findFirst('/', pos + 1);
		if (pos == str.length() || pos == String::invalid)
			return true;
		String p = str.substr(0, pos);
		if (p.length() == 2 && p[1] == ':') // skip C:/ D:/
			continue;
		if (p == "." || p == ".." || p == "/" || p == "\\")
			continue;
		if (OS::Directory::exist(p))
			continue;
		StringWide wstr = Utf8ToWchar(p.cstr());
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

bool OS::Directory::remove(const Path& path, bool recursive)
{
	if (recursive)
	{
		StringWide wstr = Utf8ToWchar(path.cstr());
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
				String p = WcharToUtf8(data.cFileName);
				remove(p, true);
			}
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
		SetCurrentDirectory(cwd);
	}
	StringWide str = Utf8ToWchar(path.cstr());
	return RemoveDirectory(str.cstr()) == TRUE;
}

bool OS::File::exist(const Path& path)
{
	StringWide str = Utf8ToWchar(path.cstr());
	DWORD ftyp = GetFileAttributes(str.cstr());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!
	return (!(ftyp & FILE_ATTRIBUTE_DIRECTORY));
}
bool OS::File::create(const Path& path)
{
	StringWide wstr = Utf8ToWchar(path.cstr());
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
bool OS::File::remove(const Path& path)
{
	StringWide str = Utf8ToWchar(path.cstr());
	return DeleteFile(str.cstr()) == TRUE;
}

String OS::File::extension(const Path& path)
{
	StringWide wstr = Utf8ToWchar(path.cstr());
	LPWSTR extension = PathFindExtension(wstr.cstr());
	String out = WcharToUtf8(extension);
	if (out.length() < 1)
		return String();
	return out.substr(1, out.length() - 1);
}

String OS::File::name(const Path& path)
{
	StringWide str = Utf8ToWchar(path.cstr());
	LPWSTR FileName = PathFindFileName(str.cstr());
	return WcharToUtf8(FileName);
}

String OS::File::basename(const Path& path)
{
	namespace fs = std::filesystem;
	fs::path stem = fs::path(path.cstr()).stem();
	std::string str = stem.string();
	return String(str.c_str(), str.size());
}

size_t OS::File::size(const Path& path)
{
	StringWide wstr = Utf8ToWchar(path.cstr());
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

Timestamp OS::File::lastWrite(const Path& path)
{
	struct _stat result;
	if (::_stat(path.cstr(), &result) != 0)
		return Timestamp::seconds(0);
	return Timestamp::seconds(result.st_mtime);
}

bool OS::File::copy(const Path& src, const Path& dst)
{
	std::filesystem::copy(src.cstr(), dst.cstr());
	return true;
}

std::vector<Path> OS::enumerate(const Path& path)
{
	const wchar_t separator = '/';
	StringWide wstr = Utf8ToWchar(path.cstr());
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

Path OS::normalize(const Path& path)
{
	WCHAR canonicalizedPath[MAX_PATH];
	StringWide wstr = Utf8ToWchar(path.cstr());
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

Path OS::executable()
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

Path OS::cwd()
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

bool OS::setcwd(const Path& path)
{
	StringWide wstr = Utf8ToWchar(path.cstr());
	return _wchdir(wstr.cstr()) == 0;
}

Path OS::temp()
{
	// TODO should cache this path somehow ?
	StringWide wstr;
	DWORD length = GetTempPath2(0, nullptr);
	AKA_ASSERT(length != 0, "Invalid temporary path");
	wstr.resize(length);
	DWORD length2 = GetTempPath2(length, wstr.cstr());
	AKA_ASSERT(length == length2 + 1, "Invalid temporary path");
	String str = WcharToUtf8(wstr.cstr()) + "/aka/";
	Path path = OS::normalize(str);
	if (!OS::Directory::exist(path))
	{
		bool created = OS::Directory::create(path);
		AKA_ASSERT(created, "Failed to create temporary dir");
	}
	return path;
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

FILE* OS::File::open(const Path& path, FileMode mode, FileType type)
{
	StringWide wstr = Utf8ToWchar(path.cstr());
	FILE* file = nullptr;
	errno_t err = _wfopen_s(&file, wstr.cstr(), fileMode(mode, type));
	if (err != 0)
	{
		char error[256];
		err = strerror_s(error, 256, err);
		Logger::error("Failed to open file ", path, " with error : ", error);
		return nullptr;
	}
	return file;
}

AlertModalMessage AlertModal(AlertModalType modalType, const char* title, const char* message)
{
	UINT type = 0;
	switch (modalType)
	{
	case AlertModalType::Information:
		type = MB_ICONINFORMATION | MB_OK;
		break;
	case AlertModalType::Question:
		type = MB_ICONQUESTION | MB_YESNO;
		break;
	case AlertModalType::Warning:
		type = MB_ICONWARNING | MB_OK;
		break;
	case AlertModalType::Error:
		type = MB_ICONERROR | MB_OK;
		break;
	}
	PlatformDevice* platform = Application::app()->platform();
	HWND handle = (HWND)platform->getNativeHandle();

	StringWide wstr = Utf8ToWchar(message);
	StringWide wstrTitle = Utf8ToWchar(title);
	int value = MessageBoxW(handle, wstr.cstr(), wstrTitle.cstr(), type);
	switch (value)
	{
	case IDYES:
		return AlertModalMessage::Yes;
	case IDNO:
		return AlertModalMessage::No;
	default:
	case IDOK:
		return AlertModalMessage::Ok;
	}
}

void* OS::Link::load(const Path& path)
{
	StringWide wstr = Utf8ToWchar(path.cstr());
	HMODULE mod = LoadLibraryW(wstr.cstr());
	if (mod == NULL)
		Logger::warn("Failed to load DLL with error : ", GetLastErrorAsString());
	return mod;
}

void* OS::Link::open(const Path& path)
{
	StringWide wstr = Utf8ToWchar(path.cstr());
	HMODULE mod = GetModuleHandleW(wstr.cstr());
	if (mod == NULL)
		Logger::warn("Failed to open DLL with error : ", GetLastErrorAsString());
	return mod;
}

void* OS::Link::getProc(void* dll, const char* proc)
{
	HMODULE mod = (HMODULE)dll;
	return GetProcAddress(mod, proc);
}

void OS::Link::free(void* module)
{
	BOOL ret = FreeLibrary((HMODULE)module);
}

};

#endif