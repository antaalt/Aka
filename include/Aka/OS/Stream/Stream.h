#pragma once

#include <stdint.h>
#include <vector>
#include <stdexcept>

#include <Aka/Core/Config.h>
#include <Aka/OS/Endian.h>
#include <Aka/Platform/Platform.h>

namespace aka {

class Stream
{
public:
	Stream();
	Stream(const Stream&) = delete;
	Stream(Stream&&) = delete;
	Stream& operator=(const Stream&) = delete;
	Stream& operator=(Stream&&) = delete;
	virtual ~Stream();

	// Serialize data and forward the cursor
	virtual void serialize(const void* data, size_t count) = 0;
	// Unserialize data and forward the cursor
	virtual void unserialize(void* data, size_t count) = 0;

	// Simply forward the cursor.
	virtual void skim(size_t count) = 0;
	// Seek the cursor
	virtual void seek(size_t position) = 0;
	// Get the size of the whole buffer.
	virtual size_t size() const = 0;
	// Get the position of the cursor.
	virtual size_t offset() = 0;
	// Set the cursor position at 0.
	virtual void rewind() = 0;
};

};