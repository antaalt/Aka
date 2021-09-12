#pragma once

#include <Aka/OS/FileSystem.h>
#include <Aka/OS/Stream/Stream.h>

#include <stdio.h>

namespace aka {

enum class FileMode 
{
	Read = (1 << 0),
	Write = (1 << 1),
	ReadWrite = Read | Write,
};

enum class FileType
{
	Binary,
	String
};

FileMode operator&(FileMode lhs, FileMode rhs);
FileMode operator|(FileMode lhs, FileMode rhs);

FILE* fopen(const Path& path, FileMode mode, FileType type);

class FileStream : public Stream
{
public:
	FileStream(const Path& path, FileMode mode, FileType type);
	~FileStream();

	void skim(size_t size) override;
	void seek(size_t position) override;
	size_t size() const override;
	size_t offset() override;
	void rewind() override;
protected:
	void readData(void* data, size_t size) override;
	void writeData(const void* data, size_t size) override;
private:
	FILE* m_file;
	FileMode m_mode;
	size_t m_length;
};

};