#include "FileSystem.h"

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
	if (m_string.size() == 0)
	{
		return rhs;
	}
	else
	{
		if (m_string.back() != '\\' && m_string.back() != '/')
			return Path(m_string + '/' + rhs.m_string);
		else
			return Path(m_string + rhs.m_string);
	}
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

};
