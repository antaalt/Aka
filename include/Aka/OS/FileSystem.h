#pragma once

#include <vector>
#include <Aka/Core/Container/String.h>
#include <Aka/Core/Container/Blob.h>

namespace aka {

struct Path
{
	Path();
	Path(const char* path);
	Path(const String& path);

	const char* cstr() const;
	size_t size() const;
	size_t length() const;

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

struct Directory
{
	// Check if a directory exist
	static bool exist(const Path &path);
	// Create a directory
	static bool create(const Path& path);
	// Remove a directory, recurse if asked
	static bool remove(const Path& path, bool recursive = false);
};

struct File
{
	// Create a file at the given path
	static bool create(const Path& path);
	// Check if a file exist at path
	static bool exist(const Path& path);
	// Remove a file from path
	static bool remove(const Path& path);
	// Get the extension of the file
	static String extension(const Path& path);
	// Get the name of the file
	static String name(const Path& path);
	// Get the size of the file
	static size_t size(const Path& path);

	// Read a file into a string
	static bool read(const Path& path, String* str);
	// Read a file into a blob
	static bool read(const Path& path, Blob* blob);

	// Write a string to a file
	static bool write(const Path& path, const char* str);
	// Write a string to a file
	static bool write(const Path& path, const String& str);
	// Write binary data to a file
	static bool write(const Path& path, const uint8_t* bytes, size_t size);
	// Write a blob to a file
	static bool write(const Path& path, const Blob& blob);
};

std::ostream& operator<<(std::ostream& os, const Path& path);

};