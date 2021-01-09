#pragma once

#include <vector>
#include <string>

namespace app {

struct Path
{
	Path();
	Path(const char* path);
	Path(const std::string& path);

	const char* c_str() const;
	const std::string &str() const;

	Path operator+(const Path& rhs) const;

private:
	std::string m_string;
};

struct Asset {
	static std::string loadString(const Path &path);
	static std::vector<uint8_t> loadBinary(const Path& path);
	static Path path(const Path& path);
};



};