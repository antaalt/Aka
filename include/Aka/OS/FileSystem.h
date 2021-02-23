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
	size_t size() const;

	explicit operator std::string& ();
	explicit operator const std::string& () const;

	Path operator+(const Path& rhs) const;
	Path &operator+=(const Path& rhs);

	bool operator==(const Path& rhs) const;
	bool operator!=(const Path& rhs) const;

	// Go to parent path
	Path up() const;

	std::string::iterator begin();
	std::string::iterator end();
	std::string::const_iterator begin() const;
	std::string::const_iterator end() const;

	// Get the current working directory
	static Path cwd();
	// Get the executable path
	static Path executable();
	// Enumerate files & folders at given path
	static std::vector<Path> enumerate(const Path& path);
	// Normalize a path
	static Path normalize(const Path& path);

private:
	std::string m_string;
};

enum class FileMode {
	Undefined = 0,
	ReadOnly = 1,
	WriteOnly = 2,
	ReadWrite = ReadOnly | WriteOnly,
};

struct File {
	File();
	File(const Path& path, FileMode mode);
	File(const File& file) = delete;
	File& operator=(const File&) = delete;
	~File();
	bool open(const Path& path, FileMode mode);
	bool close();
	bool opened() const;

	void read(void* data, size_t size);
	void write(const void* data, size_t size);
	void seek(size_t position);
	size_t length() const;
	size_t position();
private:
	FILE* m_file;
	FileMode m_mode;
	size_t m_length;
};

namespace directory {
// Check if a directory exist
bool exist(const Path &path);
// Create a directory
bool create(const Path& path);
// Remove a directory, recurse if asked
bool remove(const Path& path, bool recursive = false);
};

namespace file {
// Create a file at the given path
bool create(const Path& path);
// Check if a file exist at path
bool exist(const Path& path);
// Remove a file from path
bool remove(const Path& path);
// Get the extension of the file
std::string extension(const Path& path);
// Get the name of the file
std::string name(const Path& path);
};


std::ostream& operator<<(std::ostream& os, const Path& path);



// TODO replace by streams
struct BinaryFile {
	static std::vector<uint8_t> load(const Path& path);
	static void write(const Path& path, const std::vector<uint8_t>& bytes);
	static void write(const Path& path, const uint8_t* bytes, size_t size);
};

struct TextFile {
	static std::string load(const Path& path);
	static void write(const Path& path, const std::string& str);
	static void write(const Path& path, const char* str);
};

struct Asset {
	static Path path(const Path& path);
};



};