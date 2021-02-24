#pragma once

#include <Aka/OS/FileSystem.h>
#include <Aka/OS/Stream/Stream.h>

namespace aka {

class FileStream : public Stream
{
public:
	FileStream(const Path& path, FileMode mode);
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
	File m_file;
};

};