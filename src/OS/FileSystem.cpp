#include <Aka/OS/FileSystem.h>

#include <Aka/OS/Logger.h>
#include <Aka/Platform/PlatformBackend.h>

#include <fstream>

namespace aka {

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

Path Path::up() const
{
	const size_t separatorCount = std::count(m_string.begin(), m_string.end(), '/');
	const size_t lastCharacterOffset = m_string.size() - 1;
	size_t offset = m_string.find_last_of('/');
	if (1 == separatorCount)
	{
		if (offset != lastCharacterOffset)
			return Path(m_string.substr(0, offset + 1));
	}
	else
	{
		if (offset == lastCharacterOffset)
		{
			offset = m_string.substr(0, offset).find_last_of('/');
			return Path(m_string.substr(0, offset + 1));
		}
		else
		{
			return Path(m_string.substr(0, offset + 1));
		}
	}
	return *this;
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

std::vector<uint8_t> BinaryFile::load(const Path& path)
{
	std::vector<uint8_t> bytes;
	if (!PlatformBackend::loadBinary(path, &bytes))
	{
		Logger::error("Could not load text file " + path.str());
	}
	return bytes;
}

void BinaryFile::write(const Path& path, const std::vector<uint8_t>& bytes)
{
	if (!PlatformBackend::writeBinary(path, bytes))
	{
		Logger::error("Could not write binary file " + path.str());
	}
}

std::string TextFile::load(const Path& path)
{
	std::string str;
	if (!PlatformBackend::loadString(path, &str))
	{
		Logger::error("Could not load text file " + path.str());
	}
	return str;
}

void TextFile::write(const Path& path, const std::string& str)
{
	if (!PlatformBackend::writeString(path, str))
	{
		Logger::error("Could not write text file " + path.str());
	}
}

Path Asset::path(const Path& path)
{
	return Path::normalize(Path::cwd() + Path("asset/") + path);
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
