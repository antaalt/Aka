#include <Aka/Platform/InputBackend.h>
#include <Aka/Platform/PlatformBackend.h>
#include <Aka/Platform/Platform.h>
#include <Aka/OS/FileSystem.h>
#include <Aka/OS/Logger.h>

#if defined(AKA_PLATFORM_LINUX)

#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

namespace aka {

const unsigned int terminalColors[20] = {
	30, // ForgeroundBlack
	31, // ForegroundRed
	32, // ForegroundGreen
	33, // ForegroundYellow
	34, // ForegroundBlue
	35, // ForegroundMagenta
	36, // ForegroundCyan
	37, // ForegroundWhite
	90, // ForgeroundBrightBlack
	91, // ForegroundBrightRed
	92, // ForegroundBrightGreen
	93, // ForegroundBrightYellow
	94, // ForegroundBrightBlue 
	95, // ForegroundBrightMagenta
	96, // ForegroundBrightCyan
	97, // ForegroundBrightWhite
};

std::ostream& operator<<(std::ostream& os, Logger::Color color)
{
	if (color == Logger::Color::ForegroundNone)
		return os;
	return os << "\033[" << terminalColors[(unsigned int)color] << "m";
}

input::Key InputBackend::getKeyFromScancode(int scancode)
{
	return input::Key::Space;
}

input::KeyboardLayout InputBackend::getKeyboardLayout()
{
	return input::KeyboardLayout::Azerty;
}

bool PlatformBackend::directoryExist(const Path& path)
{
	struct stat st;
	if (stat(path.c_str(), &st) == 0)
		return ((st.st_mode & S_IFDIR) != 0);
	return false;
}
bool PlatformBackend::directoryCreate(const Path& path)
{
	size_t pos = 0;
	do
	{
		pos = path.str().find_first_of("\\/", pos + 1);
		if (pos == path.str().size())
			return true;
		std::string p = path.str().substr(0, pos);
		if (p == "." || p == ".." || p == "/" || p == "\\")
			continue;
		if (mkdir(path.str().substr(0, pos).c_str(), 0777) != 0)
		{
			if (EEXIST == errno)
				continue;
			else
				return false;
		}
	} while (pos != std::string::npos);
	return true;
}
bool PlatformBackend::directoryRemove(const Path& path, bool recursive)
{
	return rmdir(path.c_str()) == 0;
}
bool PlatformBackend::fileExist(const Path& path)
{
	return access(path.c_str(), F_OK) != -1;
}
bool PlatformBackend::fileCreate(const Path& path)
{
	std::ofstream file(path.str());
	return file.is_open();
}
bool PlatformBackend::fileRemove(const Path& path)
{
	return unlink(path.c_str()) == 0;
}

std::vector<Path> PlatformBackend::enumerate(const Path& path)
{
	DIR* dp;
	struct dirent* dirp;
	std::vector<Path> paths;
	if ((dp = opendir(path.c_str())) != NULL)
	{
		while ((dirp = readdir(dp)) != NULL)
		{
			paths.push_back(path.str() + dirp->d_name);
		}
		closedir(dp);
	}
	else
	{
		// errno
		Logger::error("Failed opening folder : ", path);
	}
	return paths;
}

Path PlatformBackend::normalize(const Path& path)
{
	// On linux, correct path should be only with '/'
	// Remove ../ & ./ aswell.
	return path;
}

Path PlatformBackend::executablePath()
{
	char result[PATH_MAX] = { 0 };
	if (readlink("/proc/self/exe", result, PATH_MAX) <= 0)
		return Path();
	return Path(result);
}

Path PlatformBackend::cwd()
{
	char path[PATH_MAX] = { 0 };
	if (getcwd(path, PATH_MAX) == nullptr)
		return Path();
	return Path(path);
}

std::string PlatformBackend::extension(const Path &path)
{
	const char *dot = strrchr(path.c_str(), '.');
    if(!dot || dot == path.c_str()) return "";
    return dot + 1;
}

std::string PlatformBackend::fileName(const Path &path)
{
	return basename(path.c_str());
}

};
#endif