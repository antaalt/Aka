#include <Aka/OS/Path.h>

#include <Aka/OS/OS.h>
#include <Aka/OS/Logger.h>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/Core/Config.h>
#include <Aka/Platform/PlatformDevice.h>

#include <algorithm>
#include <cstring>

namespace aka {

Path::Path()
{
}

Path::Path(const char* path) : m_string(path)
{
}

Path::Path(const String& path) : m_string(path)
{
}

char* Path::cstr()
{
	return m_string.cstr();
}

const char* Path::cstr() const
{
	return m_string.cstr();
}

String& Path::string()
{
	return m_string;
}
const String& Path::string() const
{
	return m_string;
}
size_t Path::size() const
{
	return m_string.length();
}
size_t Path::length() const
{
	return m_string.length();
}
void Path::reserve(size_t size)
{
	m_string.reserve(size);
}

void Path::resize(size_t size)
{
	m_string.resize(size);
}
String::Char& Path::operator[](size_t index)
{
	return m_string[index];
}
const String::Char& Path::operator[](size_t index) const
{
	return m_string[index];
}
Path Path::operator+(const Path& rhs) const
{
	Path path(*this);
	path += rhs;
	return path;
}

Path& Path::operator+=(const Path& rhs)
{
	if (m_string.length() == 0)
	{
		m_string = rhs.m_string;
		return *this;
	}
	else
	{
		if (m_string.last() != '\\' && m_string.last() != '/')
			m_string += '/' + rhs.m_string;
		else
			m_string += rhs.m_string;
	}
	return *this;
}

bool Path::operator==(const Path& rhs) const
{
	return OS::normalize(*this).m_string == OS::normalize(rhs).m_string;
}

bool Path::operator!=(const Path& rhs) const
{
	return OS::normalize(*this).m_string != OS::normalize(rhs).m_string;
}

Path Path::operator/(const Path& rhs)
{
	return Path(m_string + '/' + rhs.m_string);
}

Path& Path::append(const Path& string)
{
	m_string.append(string.cstr());
	return *this;
}

Path Path::up() const
{
	const size_t separatorCount = std::count(m_string.begin(), m_string.end(), '/');
	const size_t lastCharacterOffset = m_string.length() - 1;
	size_t offset = m_string.findLast('/');
	if (offset == String::invalid)
		return *this;
	if (1 == separatorCount)
	{
		if (offset != lastCharacterOffset)
			return Path(m_string.substr(0, offset + 1));
	}
	else
	{
		if (offset == lastCharacterOffset)
		{
			offset = m_string.substr(0, offset).findLast('/');
			return Path(m_string.substr(0, offset + 1));
		}
		else
		{
			return Path(m_string.substr(0, offset + 1));
		}
	}
	return *this;
}

char* Path::begin()
{
	return m_string.begin();
}

char* Path::end()
{
	return m_string.end();
}

const char* Path::begin() const
{
	return m_string.begin();
}

const char* Path::end() const
{
	return m_string.end();
}

std::ostream& operator<<(std::ostream& os, const Path& path)
{
	return os << path.cstr();
}

};
