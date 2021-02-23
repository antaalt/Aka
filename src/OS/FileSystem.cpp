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

size_t Path::size() const
{
	return m_string.size();
}

Path::operator std::string& ()
{
	return m_string;
}

Path::operator const std::string& () const
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

std::string::iterator Path::begin()
{
	return m_string.begin();
}

std::string::iterator Path::end()
{
	return m_string.end();
}

std::string::const_iterator Path::begin() const
{
	return m_string.begin();
}

std::string::const_iterator Path::end() const
{
	return m_string.end();
}

Path Asset::path(const Path& path)
{
	return Path::normalize(Path::cwd() + Path("asset/") + path);
}

std::ostream& operator<<(std::ostream& os, const Path& path)
{
	return os << path.c_str();
}

};
