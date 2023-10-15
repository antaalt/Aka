#pragma once

#include <vector>
#include <Aka/Core/Container/String.h>
#include <Aka/Core/Container/Blob.h>
#include <Aka/OS/Time.h>

namespace aka {

struct Path
{
	Path();
	Path(const char* path);
	Path(const String& path);
	Path(const Path&) = default;
	Path(Path&&) = default;
	Path& operator=(const Path&) = default;
	Path& operator=(Path&&) = default;
	~Path() = default;

	char* cstr();
	const char* cstr() const;
	size_t size() const;
	size_t length() const;
	void reserve(size_t size);
	void resize(size_t size);

	Path operator+(const Path& rhs) const;
	Path &operator+=(const Path& rhs);

	bool operator==(const Path& rhs) const;
	bool operator!=(const Path& rhs) const;

	Path operator/(const Path& rhs);

	Path& append(const Path& string);

	// Go to parent path
	Path up() const;

	char* begin();
	char* end();
	const char* begin() const;
	const char* end() const;
private:
	String m_string;
};

std::ostream& operator<<(std::ostream& os, const Path& path);

};