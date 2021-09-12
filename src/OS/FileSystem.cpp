#include <Aka/OS/FileSystem.h>

#include <Aka/OS/Logger.h>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/Core/Debug.h>
#include <Aka/Platform/PlatformBackend.h>

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

const char* Path::cstr() const
{
	return m_string.cstr();
}

size_t Path::size() const
{
	return m_string.length();
}
size_t Path::length() const
{
	return m_string.length();
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
	return Path::normalize(*this).m_string == Path::normalize(rhs).m_string;
}

bool Path::operator!=(const Path& rhs) const
{
	return Path::normalize(*this).m_string != Path::normalize(rhs).m_string;
}

Path Path::up() const
{
	const size_t separatorCount = std::count(m_string.begin(), m_string.end(), '/');
	const size_t lastCharacterOffset = m_string.length() - 1;
	size_t offset = m_string.findLast('/');
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

bool File::read(const Path& path, String* str)
{
	FILE* file = fopen(path, FileMode::Read, FileType::String);
	if (file == nullptr)
		return false;
	int error = fseek(file, 0L, SEEK_END);
	size_t size = ftell(file);
	rewind(file);
	*str = String(size);
	if (fread(str->cstr(), 1, str->length(), file) != str->length())
		return false;
	fclose(file);
	return true;
}

bool File::read(const Path& path, Blob* blob)
{
	FILE* file = fopen(path, FileMode::Read, FileType::Binary);
	if (file == nullptr)
		return false;
	int error = fseek(file, 0L, SEEK_END);
	size_t size = ftell(file);
	rewind(file);
	*blob = Blob(size);
	if (fread(blob->data(), 1, blob->size(), file) != blob->size())
		return false;
	fclose(file);
	return true;
}

bool File::write(const Path& path, const char* str)
{
	FILE* file = fopen(path, FileMode::Write, FileType::String);
	if (file == nullptr)
		return false;
	size_t length = strlen(str);
	if (length != fwrite(str, 1, length, file))
		return false;
	fclose(file);
	return true;
}

bool File::write(const Path& path, const String& str)
{
	return write(path, str.cstr());
}

bool File::write(const Path& path, const uint8_t* bytes, size_t size)
{
	FILE* file = fopen(path, FileMode::Write, FileType::Binary);
	if (file == nullptr)
		return false;
	if (size != fwrite(bytes, 1, size, file))
		return false;
	fclose(file);
	return true;
}

bool File::write(const Path& path, const Blob& blob)
{
	return write(path, blob.data(), blob.size());
}

std::ostream& operator<<(std::ostream& os, const Path& path)
{
	return os << path.cstr();
}

};
