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

std::vector<uint8_t> BinaryFile::load(const Path& path)
{
	std::basic_ifstream<uint8_t> ifs(path.str(), std::ios::binary);
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
	std::basic_ofstream<uint8_t> ofs(path.str(), std::ios::binary);
	if (!ofs)
	{
		Logger::error("Could not write binary file ", path);
		return;
	}
	ofs.write(bytes, size);
}

std::string TextFile::load(const Path& path)
{
	std::ifstream ifs(path.str());
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
	std::ofstream ofs(path.str());
	if (!ofs)
	{
		Logger::error("Could not write text file ", path);
		return;
	}
	ofs << str;
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
		return "r";
	case FileMode::WriteOnly:
		return "w";
	default:
	case FileMode::ReadWrite:
		return "rw";
	}
}

FILE* fopen(const Path& path, FileMode mode)
{
	return fopen(path.c_str(), fileMode(mode));
}

};

#endif