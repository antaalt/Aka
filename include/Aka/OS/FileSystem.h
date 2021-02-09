#pragma once

#include <vector>
#include <string>

namespace aka {

struct Path
{
	Path();
	Path(const char* path);
	Path(const std::string& path);

	const char* c_str() const;
	const std::string &str() const;

	Path operator+(const Path& rhs) const;
	Path &operator+=(const Path& rhs);

	bool operator==(const Path& rhs) const;
	bool operator!=(const Path& rhs) const;

	static std::string extension(const Path &path);
	static std::string name(const Path& path);
	static Path cwd();
	static Path executable();
	static std::vector<Path> enumerate(const Path& path);
	static Path normalize(const Path& path);

private:
	std::string m_string;
};

std::ostream& operator<<(std::ostream& os, const Path& path);

namespace directory {
bool exist(const Path &path);
bool create(const Path& path);
bool remove(const Path& path);
};

namespace file {
bool create(const Path& path);
bool exist(const Path& path);
bool remove(const Path& path);
};

struct Asset {
	static std::string loadString(const Path &path);
	static std::vector<uint8_t> loadBinary(const Path& path);
	static Path path(const Path& path);
};



};