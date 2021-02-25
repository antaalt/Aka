#include <Aka/OS/FileSystem.h>

#include <Aka/OS/Logger.h>
#include <Aka/Core/Debug.h>
#include <Aka/Platform/PlatformBackend.h>

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

File::File() :
	m_file(nullptr),
	m_mode(FileMode::ReadOnly),
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

std::string File::readString(const Path& path)
{
	File file(path, FileMode::ReadOnly);
	if (!file.opened())
		throw std::runtime_error("Failed to load string : " + path.str());
	std::string str;
	str.resize(file.length());
	if (!file.read(str.data(), str.length()))
		throw std::runtime_error("Failed to read string : " + path.str());
	return str;
}

std::vector<uint8_t> File::readBinary(const Path& path)
{
	File file(path, FileMode::ReadOnly);
	if (!file.opened())
		throw std::runtime_error("Failed to load binary : " + path.str());
	std::vector<uint8_t> bytes;
	bytes.resize(file.length());
	if (!file.read(bytes.data(), bytes.size()))
		throw std::runtime_error("Failed to read binary : " + path.str());
	return bytes;
}

bool File::writeString(const Path& path, const char* str)
{
	File file(path, FileMode::WriteOnly);
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
	File file(path, FileMode::WriteOnly);
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

std::ostream& operator<<(std::ostream& os, const Path& path)
{
	return os << path.c_str();
}

};
