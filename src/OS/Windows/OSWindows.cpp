#include <Aka/OS/OS.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Core/Application.h>

#include "WindowsCommon.hpp"

#if defined(AKA_PLATFORM_WINDOWS)

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
	BackwardToForwardSlash(str);
	RemoveNeighborDuplicateSlash(str);
	// Windows require last string to own a slash
	if (str.last() != '/')
	{
		str.append('/');
	}
	do
	{
		pos = str.findFirst('/', pos + 1);
		if (pos == str.length() || pos == String::invalid)
			return true;
		String p = str.substr(0, pos);
		if (p.length() == 2 && p[1] == ':') // skip C:/ D:/
			continue;
		if (p == "." || p == ".." || p == "/")
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
	struct _stat result;
	if (::_wstat(wstr.cstr(), &result) != 0)
		return 0;
	return result.st_size;
}

Timestamp OS::File::lastWrite(const Path& path)
{
	StringWide wstr = Utf8ToWchar(path.cstr());
	struct _stat result;
	if (::_wstat(wstr.cstr(), &result) != 0)
		return Timestamp::seconds(0);
	return Timestamp::seconds(result.st_mtime);
}

bool OS::File::copy(const Path& src, const Path& dst)
{
	std::filesystem::copy(src.cstr(), dst.cstr());
	return true;
}

Vector<Path> OS::enumerate(const Path& path)
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
	Vector<Path> paths;
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
				paths.append(str + '/');
			}
			else
			{
				// File
				String str = WcharToUtf8(data.cFileName);
				paths.append(str);
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
		BackwardToForwardSlash(str);
		return str;
	}
	else
	{
		Logger::error("OS::normalize failed: ", GetLastErrorAsString());
		return path;
	}
}

Path OS::getFullPath(const Path& path)
{
	WCHAR fullPath[MAX_PATH];
	StringWide wstr = Utf8ToWchar(path.cstr());
	DWORD res = GetFullPathName(wstr.cstr(), MAX_PATH, fullPath, NULL);
	if (res == 0)
	{
		Logger::error("OS::getFullPath failed: ", GetLastErrorAsString());
		return Path();
	}
	String str = WcharToUtf8(fullPath);
	return str;
}

Path OS::relative(const Path& path, const Path& from)
{
	WCHAR relativePath[MAX_PATH] = L"";
	StringWide towstr = Utf8ToWchar(OS::getFullPath(path).cstr()); // MAX_PATH
	StringWide fromwstr = Utf8ToWchar(OS::getFullPath(from).cstr()); // MAX_PATH
	BOOL res = PathRelativePathTo(
		relativePath,
		fromwstr.cstr(), 
		FILE_ATTRIBUTE_DIRECTORY,
		towstr.cstr(),
		FILE_ATTRIBUTE_NORMAL
	);
	if (res != TRUE)
	{
		Logger::error("OS::relative failed: ", GetLastErrorAsString());
		return Path();
	}
	String finalPath = WcharToUtf8(relativePath);
	BackwardToForwardSlash(finalPath);
	return Path(finalPath);
}

Path OS::executable()
{
	WCHAR path[MAX_PATH]{};
	if (GetModuleFileName(NULL, path, MAX_PATH) == 0)
		return Path();
	String str = WcharToUtf8(path);
	BackwardToForwardSlash(str);
	return Path(str);
}

Path OS::cwd()
{
	WCHAR path[MAX_PATH] = { 0 };
	if (GetCurrentDirectory(MAX_PATH, path) == 0)
		return Path();
	String str = WcharToUtf8(path);
	BackwardToForwardSlash(str);
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
	DWORD length = GetTempPath(0, nullptr);
	AKA_ASSERT(length != 0, "Invalid temporary path");
	wstr.resize(length);
	DWORD length2 = GetTempPath(length, wstr.cstr());
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
		type = MB_ICONWARNING | MB_CANCELTRYCONTINUE;
		break;
	case AlertModalType::Error:
		type = MB_ICONERROR | MB_ABORTRETRYIGNORE;
		break;
	case AlertModalType::Critical: // unrecoverable
		type = MB_ICONERROR | MB_OK;
		break;
	}
	PlatformWindow* window = Application::app()->window();
	HWND handle = (HWND)window->getNativeHandle();

	StringWide wstr = Utf8ToWchar(message);
	StringWide wstrTitle = Utf8ToWchar(title);
	int value = MessageBoxW(handle, wstr.cstr(), wstrTitle.cstr(), type);
	switch (value)
	{
	case IDYES:
		return AlertModalMessage::Yes;
	case IDNO:
		return AlertModalMessage::No;
	case IDIGNORE:
		return AlertModalMessage::Ignore;
	case IDRETRY:
		return AlertModalMessage::Retry;
	case IDABORT:
		return AlertModalMessage::Abort;
	case IDCANCEL:
		return AlertModalMessage::Cancel;
	default:
	case IDOK:
		return AlertModalMessage::Ok;
	}
}

OS::Library::Library(const Path& path) :
	m_handle(nullptr)
{
	StringWide wstr = Utf8ToWchar(path.cstr());
	HMODULE mod = LoadLibraryW(wstr.cstr());
	if (mod == NULL)
	{
		Logger::warn("Failed to load DLL '", path.cstr(), "' with error : ", GetLastErrorAsString());
	}
	m_handle = static_cast<LibraryHandle>(mod);
}

OS::Library::~Library()
{
	if (m_handle)
	{
		BOOL ret = FreeLibrary(static_cast<HMODULE>(m_handle));
		if (!ret)
		{
			Logger::warn("Failed to free DLL with error : ", GetLastErrorAsString());
		}
	}
}

OS::ProcessHandle OS::Library::getProcess(const char* _process)
{
	AKA_ASSERT(m_handle != nullptr, "GetProcess on null pointer");
	FARPROC proc = GetProcAddress(static_cast<HMODULE>(m_handle), _process);

	return static_cast<OS::ProcessHandle>(proc);
}

};

#endif