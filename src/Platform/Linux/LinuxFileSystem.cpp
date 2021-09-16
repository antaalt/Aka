#include <Aka/OS/FileSystem.h>
#include <Aka/Platform/Platform.h>
#include <Aka/OS/Logger.h>

#if defined(AKA_PLATFORM_LINUX)

#include <stdio.h>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

namespace aka {

bool Directory::exist(const Path& path)
{
	struct stat st;
	if (stat(path.cstr(), &st) == 0)
		return ((st.st_mode & S_IFDIR) != 0);
	return false;
}

bool Directory::create(const Path& path)
{
	size_t pos = 0;
	do
	{
		pos = path.str().findFirst('/', pos + 1);
		if (pos == path.str().length())
			return true;
		std::string p = path.str().substr(0, pos);
		if (p == "." || p == ".." || p == "/" || p == "\\")
			continue;
		if (mkdir(path.str().substr(0, pos).cstr(), 0777) != 0)
		{
			if (EEXIST == errno)
				continue;
			else
				return false;
		}
	} while (pos != std::string::npos);
	return true;
}

bool Directory::remove(const Path& path, bool recursive)
{
	return rmdir(path.cstr()) == 0;
}

bool File::exist(const Path& path)
{
	return access(path.cstr(), F_OK) != -1;
}
bool File::create(const Path& path)
{
	std::ofstream f(path.cstr());
	return f.is_open();
}
bool File::remove(const Path& path)
{
	return unlink(path.cstr()) == 0;
}

String File::extension(const Path& path)
{
	const char* dot = strrchr(path.cstr(), '.');
	if (!dot || dot == path.cstr()) return "";
	return dot + 1;
}

String File::name(const Path& path)
{
	return basename(path.cstr());
}
size_t File::size(const Path& path)
{
	struct stat st;
	if (::stat(filename, &st) != 0)
		return 0;
	return st.st_size;
}

Time::Unit File::lastWrite(const Path& path)
{
	struct stat result;
	if (::stat(path.cstr(), &result) != 0)
		return Time::Unit::milliseconds(0);
	return Time::Unit::milliseconds(result.st_mtime * 1000);
}

std::vector<Path> Path::enumerate(const Path& path)
{
	DIR* dp;
	struct dirent* dirp;
	std::vector<Path> paths;
	if ((dp = opendir(path.cstr())) != NULL)
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

const char* fileMode(FileMode mode, FileType type)
{
	switch (type)
	{
	case aka::FileType::Binary:
		if (mode == FileMode::Read)
			return "rb";
		else if (mode == FileMode::Write)
			return "wb";
		else if (mode == FileMode::ReadWrite)
			return "rwb";
		break;
	case aka::FileType::String:
		if (mode == FileMode::Read)
			return "r";
		else if (mode == FileMode::Write)
			return "w";
		else if (mode == FileMode::ReadWrite)
			return "rw";
		break;
	default:
		break;
	}
	return "";
}

FILE* fopen(const Path& path, FileMode mode, FileType type)
{
	return ::fopen(path.cstr(), fileMode(mode, type));
}

};

#endif