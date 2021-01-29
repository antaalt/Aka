#include "../InputBackend.h"
#include "../PlatformBackend.h"
#include "../Platform.h"
#include "../../OS/FileSystem.h"

#if defined(AKA_LINUX)

#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

namespace aka {

input::Key InputBackend::getKeyFromScancode(int scancode)
{
	return input::Key::Space;
}

input::KeyboardLayout InputBackend::getKeyboardLayout()
{
	return input::KeyboardLayout::Default;
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
		if (pos == path.size())
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
bool PlatformBackend::directoryRemove(const Path& path)
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

};

#endif