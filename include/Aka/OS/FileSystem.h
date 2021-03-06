#pragma once

#include <vector>
#include <Aka/Core/String.h>

namespace aka {

struct Path
{
	Path();
	Path(const char* path);
	Path(const String& path);

	const char* cstr() const;
	const String &str() const;
	size_t size() const;
	size_t length() const;

	explicit operator String& ();
	explicit operator const String& () const;

	Path operator+(const Path& rhs) const;
	Path &operator+=(const Path& rhs);

	bool operator==(const Path& rhs) const;
	bool operator!=(const Path& rhs) const;

	// Go to parent path
	Path up() const;

	char* begin();
	char* end();
	const char* begin() const;
	const char* end() const;

	// Get the current working directory
	static Path cwd();
	// Get the executable path
	static Path executable();
	// Enumerate files & folders at given path
	static std::vector<Path> enumerate(const Path& path);
	// Normalize a path
	static Path normalize(const Path& path);

private:
	String m_string;
};

enum class FileMode {
	ReadOnly,
	WriteOnly,

	ReadWrite,
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

	bool read(void* data, size_t size);
	bool write(const void* data, size_t size);
	bool seek(size_t position);
	size_t length() const;
	size_t position();

	static std::string readString(const Path& path);
	static std::vector<uint8_t> readBinary(const Path& path);
	static bool writeString(const Path& path, const char* str);
	static bool writeString(const Path& path, const std::string& str);
	static bool writeBinary(const Path& path, const uint8_t* bytes, size_t size);
	static bool writeBinary(const Path& path, const std::vector<uint8_t>& bytes);
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
String extension(const Path& path);
// Get the name of the file
String name(const Path& path);
};

std::ostream& operator<<(std::ostream& os, const Path& path);

struct Asset {
	static Path path(const Path& path);
};



};