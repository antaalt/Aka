#include <Aka/OS/OS.h>

namespace aka {

bool OS::File::read(const Path& path, String* str)
{
	FILE* file = open(path, FileMode::Read, FileType::String);
	if (file == nullptr)
		return false;
	String buf(4096, '\0');
	size_t read = 0;
	while ((read = fread(buf.cstr(), 1, buf.length(), file)) == buf.length())
		str->append(buf.cstr(), read);
	str->append(buf.cstr(), read);
	fclose(file);
	return true;
}

bool OS::File::read(const Path& path, Blob* blob)
{
	FILE* file = open(path, FileMode::Read, FileType::Binary);
	if (file == nullptr)
		return false;
	int error = fseek(file, 0L, SEEK_END);
	size_t size = ftell(file);
	rewind(file);
	*blob = Blob(size);
	if (fread(blob->data(), 1, blob->size(), file) != blob->size())
	{
		fclose(file);
		return false;
	}
	fclose(file);
	return true;
}

bool OS::File::write(const Path& path, const char* str)
{
	FILE* file = open(path, FileMode::Write, FileType::String);
	if (file == nullptr)
		return false;
	size_t length = strlen(str);
	if (length != fwrite(str, 1, length, file))
	{
		fclose(file);
		return false;
	}
	fclose(file);
	return true;
}

bool OS::File::write(const Path& path, const String& str)
{
	return write(path, str.cstr());
}

bool OS::File::write(const Path& path, const uint8_t* bytes, size_t size)
{
	FILE* file = open(path, FileMode::Write, FileType::Binary);
	if (file == nullptr)
		return false;
	if (size != fwrite(bytes, 1, size, file))
	{
		fclose(file);
		return false;
	}
	fclose(file);
	return true;
}

bool OS::File::write(const Path& path, const Blob& blob)
{
	return write(path, blob.data(), blob.size());
}

};