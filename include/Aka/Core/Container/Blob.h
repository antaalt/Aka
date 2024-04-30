#pragma once

#include <stdint.h>

#include <Aka/Memory/Memory.h>

namespace aka {

class Blob final
{
public:
	Blob();
	explicit Blob(size_t size);
	Blob(const void* bytes, size_t size);
	Blob(const Blob& blob);
	Blob& operator=(const Blob& blob);
	Blob(Blob&& blob) noexcept;
	Blob& operator=(Blob&& blob) noexcept;
	~Blob();

	void* data();
	const void* data() const;
	size_t size() const;
private:
	byte_t* m_bytes;
	size_t m_size;
};

};