#pragma once

#include <Aka/OS/Path.h>
#include <Aka/OS/Logger.h>
#include <Aka/OS/Stream/FileStream.h>

#include <vector>

namespace aka {

struct OS
{
	// Normalize a path for aka
	static Path normalize(const Path& path);
	// Get a full path from a relative path
	static Path getFullPath(const Path& path);
	// Get the path of the executable.
	static Path executable();
	// Get the current working directory
	static Path cwd();
	// Set the current working directory
	static bool setcwd(const Path& path);
	// Get the temporary folder directory
	static Path temp();
	// Enumerate all path in a directory.
	static std::vector<Path> enumerate(const Path& path);

	struct File
	{
		// Create a file
		static bool create(const Path& path);
		// Check if a file exist
		static bool exist(const Path& path);
		// Remove a file
		static bool remove(const Path& path);
		// Get the size of a file if it exist
		static size_t size(const Path& path);
		// Get the last write timestamp
		static Timestamp lastWrite(const Path& path);
		// Copy a file
		static bool copy(const Path& src, const Path& dst);

		// Get the name of the file
		static String name(const Path& path);
		// Get the base name of the file without the extension
		static String basename(const Path& path);
		// Get the extension of the file
		static String extension(const Path& path);

		// Read a file into a string
		static bool read(const Path& path, String* str);
		// Read a file into a blob
		static bool read(const Path& path, Blob* blob);

		// Write a string to a file
		static bool write(const Path& path, const char* str);
		// Write a string to a file
		static bool write(const Path& path, const String& str);
		// Write binary data to a file
		static bool write(const Path& path, const void* bytes, size_t size);
		// Write a blob to a file
		static bool write(const Path& path, const Blob& blob);

		// Open the file and get a handle to it.
		static FILE* open(const Path& path, FileMode mode, FileType type);
	};

	struct Directory
	{
		// Create a directory recursively
		static bool create(const Path& path);
		// Check if a directory exist
		static bool exist(const Path& path);
		// Remove a directory.
		static bool remove(const Path& path, bool recurse = false);
	};

	using ProcessHandle = void*;
	using LibraryHandle = void*;

	struct Library
	{
		Library() : m_handle(nullptr) {}
		Library(const Path& path);
		Library(const Library&) = delete;
		Library(Library&& e) { std::swap(m_handle, e.m_handle); }
		Library& operator=(const Library&) = delete;
		Library& operator=(Library&& e) { std::swap(m_handle, e.m_handle); return *this; }
		~Library();

		// Look into common places for a given library.
		static Path getLibraryPath(const char* _name);

		// Get a process from the library
		ProcessHandle getProcess(const char* _process);
		// Check if library is loaded
		bool isLoaded() const { return m_handle != nullptr; }
	private:
		LibraryHandle m_handle;
	};
};

std::ostream& operator<<(std::ostream& os, Logger::Color color);

enum class AlertModalType
{
	Information,
	Question,
	Warning,
	Error,
};

enum class AlertModalMessage
{
	Yes,
	No,
	Ok,
};

// Open a modal with a display message
// Blocking.
AlertModalMessage AlertModal(AlertModalType type, const char* title, const char* message);

}; // namespace aka