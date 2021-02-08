#include "FileSystem.h"
#include "../Platform/PlatformBackend.h"

#include <fstream>

namespace aka {

std::vector<uint8_t> readBinary(const Path& path)
{
	std::basic_ifstream<uint8_t> ifs(path.str(), std::ios::binary);
	if (!ifs)
		throw std::runtime_error("Could not load file " + path.str());
	return std::vector<uint8_t>((std::istreambuf_iterator<uint8_t>(ifs)), (std::istreambuf_iterator<uint8_t>()));
}

void writeBinary(const Path& path, const std::vector<uint8_t>& bytes)
{
	std::basic_ofstream<uint8_t> ofs(path.str(), std::ios::binary);
	ofs.write(bytes.data(), bytes.size());
}

std::string readString(const Path& path)
{
	std::ifstream ifs(path.str());
	if (!ifs)
		throw std::runtime_error("Could not load file " + path.str());
	return std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
}

void writeString(const Path& path, const std::string& text)
{
	std::ofstream ofs(path.str());
	ofs.write(text.data(), text.size());
}

Path::Path()
{
}

Path::Path(const char* path) : m_string(path)
{
}

Path::Path(const std::string& path) : m_string(path)
{
}

const char* Path::c_str() const
{
	return m_string.c_str();
}

const std::string& Path::str() const
{
	return m_string;
}

Path Path::operator+(const Path& rhs) const
{
	Path path(*this);
	path += rhs;
	return path;
}

Path& Path::operator+=(const Path& rhs)
{
	if (m_string.size() == 0)
	{
		m_string = rhs.m_string;
		return *this;
	}
	else
	{
		if (m_string.back() != '\\' && m_string.back() != '/')
			m_string += '/' + rhs.m_string;
		else
			m_string += rhs.m_string;
	}
	return *this;
}

bool Path::operator==(const Path& rhs) const
{
	return Path::normalize(*this).str() == Path::normalize(rhs).str();
}

bool Path::operator!=(const Path& rhs) const
{
	return Path::normalize(*this).str() != Path::normalize(rhs).str();
}

std::string Path::extension(const Path& path)
{
	return PlatformBackend::extension(path);
}

std::string Path::name(const Path& path)
{
	return PlatformBackend::fileName(path);
}

Path Path::cwd()
{
	return PlatformBackend::cwd();
}

Path Path::executable()
{
	return PlatformBackend::executablePath();
}

std::vector<Path> Path::enumerate(const Path& path)
{
	return PlatformBackend::enumerate(path);
}

Path Path::normalize(const Path& path)
{
	return PlatformBackend::normalize(path);
}

std::string Asset::loadString(const Path& path)
{
	return readString(Path("../asset/") + path);
}

std::vector<uint8_t> Asset::loadBinary(const Path& path)
{
	return readBinary(Path("../asset/") + path);
}

Path Asset::path(const Path& path)
{
	return Path("../asset/") + path;
}

bool directory::exist(const Path& path)
{
	return PlatformBackend::directoryExist(path);
}

bool directory::create(const Path& path)
{
	return PlatformBackend::directoryCreate(path);
}

bool directory::remove(const Path& path)
{
	return PlatformBackend::directoryRemove(path);
}

bool file::create(const Path& path)
{
	return PlatformBackend::fileCreate(path);
}

bool file::exist(const Path& path)
{
	return PlatformBackend::fileExist(path);
}

bool file::remove(const Path& path)
{
	return PlatformBackend::fileRemove(path);
}

std::ostream& operator<<(std::ostream& os, const Path& path)
{
	return os << path.c_str();
}

};
