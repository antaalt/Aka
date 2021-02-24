#include <Aka/OS/FileSystem.h>
#include <Aka/Platform/Platform.h>
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

bool directory::exist(const Path& path)
{
	struct stat st;
	if (stat(path.c_str(), &st) == 0)
		return ((st.st_mode & S_IFDIR) != 0);
	return false;
}

bool directory::create(const Path& path)
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

bool directory::remove(const Path& path, bool recursive)
{
	return rmdir(path.c_str()) == 0;
}

bool file::exist(const Path& path)
{
	return access(path.c_str(), F_OK) != -1;
}
bool file::create(const Path& path)
{
	std::ofstream file(path.str());
	return file.is_open();
}
bool file::remove(const Path& path)
{
	return unlink(path.c_str()) == 0;
}

std::string file::extension(const Path& path)
{
	const char* dot = strrchr(path.c_str(), '.');
	if (!dot || dot == path.c_str()) return "";
	return dot + 1;
}

std::string file::name(const Path& path)
{
	return basename(path.c_str());
}

std::vector<Path> Path::enumerate(const Path& path)
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

Path Path::normalize(const Path& path)
{
	// On linux, correct path should be only with '/'
	// Remove ../ & ./ aswell.
	return path;
}

Path Path::executable()
{
	char result[PATH_MAX] = { 0 };
	if (readlink("/proc/self/exe", result, PATH_MAX) <= 0)
		return Path();
	return Path(result);
}

Path Path::cwd()
{
	char path[PATH_MAX] = { 0 };
	if (getcwd(path, PATH_MAX) == nullptr)
		return Path();
	return Path(path);
}

const char* fileMode(FileMode mode)
{
	switch (mode)
	{
	case FileMode::ReadOnly:
		return "rb";
	case FileMode::WriteOnly:
		return "wb";
	default:
	case FileMode::ReadWrite:
		return "rwb";
	}
}

const char* fileMode(FileMode mode, FileType type)
{
	switch (mode)
	{
	case FileMode::ReadOnly:
		return type == FileType::Binary ? "rb" : L"r";
	case FileMode::WriteOnly:
		return type == FileType::Binary ? "wb" : L"w";
	default:
	case FileMode::ReadWrite:
		return type == FileType::Binary ? "rwb" : L"rw";
	}
}

FILE* fopen(const Path& path, FileMode mode, FileType type)
{
	return fopen(path.c_str(), fileMode(mode, type));
}

};

#endif