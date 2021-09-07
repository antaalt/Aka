#include <Aka/OS/FileSystem.h>

#include <Aka/OS/Logger.h>
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

const String& Path::str() const
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

Path::operator String& ()
{
	return m_string;
}

Path::operator const String& () const
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
	return Path::normalize(*this).str() == Path::normalize(rhs).str();
}

bool Path::operator!=(const Path& rhs) const
{
	return Path::normalize(*this).str() != Path::normalize(rhs).str();
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

File::File() :
	m_file(nullptr),
	m_mode(FileMode::Read),
	m_length(0)
{
}

File::File(const Path& path, FileMode mode) :
	File()
{
	if (!open(path, mode))
		Logger::error("Failed to open file : ", path);
}

File::~File()
{
	if (m_file != nullptr)
		close();
}

// Implementation defined fopen
FILE* fopen(const Path& path, FileMode mode);

bool File::open(const Path& path, FileMode mode)
{
	m_file = fopen(path, mode);
	if (m_file == nullptr)
		return false;
	m_mode = mode;
	fseek(m_file, 0L, SEEK_END);
	m_length = ftell(m_file);
	rewind(m_file);
	return true;
}

bool File::close() 
{
	int error = fclose(m_file);
	m_file = nullptr;
	m_length = 0;
	return error != 0;
}

bool File::opened() const
{
	return m_file != nullptr;
}

bool File::read(void* data, size_t size)
{
	size_t length = fread(data, 1, size, m_file);
	return length == size;
}

bool File::write(const void* data, size_t size)
{
	size_t length = fwrite(data, 1, size, m_file);
	return length == size;
}

bool File::seek(size_t position)
{
	int error = fseek(m_file, (long)position, SEEK_SET);
	return error == 0;
}

size_t File::length() const
{
	return m_length;
}

size_t File::position()
{
	return ftell(m_file);
}

FileMode File::mode() const
{
	return m_mode;
}

std::string File::readString(const Path& path)
{
	File file(path, FileMode::Read);
	if (!file.opened())
		return std::string();
	std::string str;
	str.resize(file.length());
	if (!file.read(str.data(), str.length()))
		return std::string();
	return str;
}

std::vector<uint8_t> File::readBinary(const Path& path)
{
	File file(path, FileMode::Read);
	if (!file.opened())
		return std::vector<uint8_t>();
	std::vector<uint8_t> bytes;
	bytes.resize(file.length());
	if (!file.read(bytes.data(), bytes.size()))
		return std::vector<uint8_t>();
	return bytes;
}

bool File::writeString(const Path& path, const char* str)
{
	File file(path, FileMode::Write);
	if (!file.opened())
		return false;
	return file.write(str, strlen(str));
}

bool File::writeString(const Path& path, const std::string& str)
{
	return writeString(path, str.c_str());
}

bool File::writeBinary(const Path& path, const uint8_t* bytes, size_t size)
{
	File file(path, FileMode::Write);
	if (!file.opened())
		return false;
	return file.write(bytes, size);
}

bool File::writeBinary(const Path& path, const std::vector<uint8_t>& bytes)
{
	return writeBinary(path, bytes.data(), bytes.size());
}

Path Asset::path(const Path& path)
{
	return Path::normalize(Path::cwd() + Path("asset/") + path);
}

FileMode operator&(FileMode lhs, FileMode rhs)
{
	return (FileMode)((int)lhs & (int)rhs);
}

FileMode operator|(FileMode lhs, FileMode rhs)
{
	return (FileMode)((int)lhs | (int)rhs);
}

std::ostream& operator<<(std::ostream& os, const Path& path)
{
	return os << path.cstr();
}

};
