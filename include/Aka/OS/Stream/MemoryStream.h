#pragma once

#include <Aka/OS/Stream/Stream.h>
#include <Aka/Core/Container/Blob.h>
#include <Aka/Core/Container/Vector.h>

namespace aka {

class MemoryStream : public Stream
{
public:
	MemoryStream(size_t size);

	void skim(size_t count) override;
	void seek(size_t position) override;
	size_t size() const override;
	size_t offset() override;
	void rewind() override;
protected:
	size_t m_size;		// Size of the buffer.
	size_t m_offset;	// Current offset position.
};

class MemoryReaderStream : public MemoryStream
{
public:
	MemoryReaderStream(const uint8_t* bytes, size_t size);
	MemoryReaderStream(const Vector<uint8_t>& bytes);
	MemoryReaderStream(const Blob& blob);
	~MemoryReaderStream();
protected:
	void read(void* data, size_t size) override;
	void write(const void* data, size_t size) override;
private:
	const uint8_t* m_bytes;
};

class MemoryWriterStream : public MemoryStream
{
public:
	MemoryWriterStream(Vector<uint8_t>& bytes);
	~MemoryWriterStream();
protected:
	void read(void* data, size_t size) override;
	void write(const void* data, size_t size) override;
private:
	Vector<byte_t>& m_bytes;

};

};