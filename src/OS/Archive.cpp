#include <Aka/OS/Archive.h>

namespace aka {

BinaryArchive::BinaryArchive(bool isReading, Endianess endianess) :
	m_endianess(endianess),
	m_reading(isReading)
{
}

void BinaryArchive::parse(String& value)
{
	uint32_t size = static_cast<uint32_t>(value.size());
	parse<uint32_t>(size);
	if (isReading())
		value.resize(size);
	parseBlob(value.data(), value.size());
	value[value.size()] = '\0';
}

void BinaryArchive::parse(point3f& value)
{
	for (uint32_t i = 0; i < 3; ++i)
	{
		parse<float>(value.data[i]);
	}
}

void BinaryArchive::parse(color4f& value)
{
	for (uint32_t i = 0; i < 4; ++i)
	{
		parse<float>(value.data[i]);
	}
}

void BinaryArchive::parse(mat4f& value)
{
	for (uint32_t i = 0; i < 4; ++i)
	{
		for (uint32_t j = 0; j < 4; ++j)
		{
			parse<float>(value.cols[i][j]);
		}
	}
}
BinaryArchiveReader::BinaryArchiveReader(Stream& stream, Endianess endianess) :
	BinaryArchive(true, endianess),
	m_stream(stream)
{
}
bool BinaryArchiveReader::expectBlob(const void* expected, size_t size)
{
	m_cache.resize(size);
	m_stream.read(m_cache.data(), size);
	return Memory::compare(m_cache.data(), expected, size) == 0;
}
void BinaryArchiveReader::parseBlob(void* value, size_t size)
{
	m_stream.read(value, size);
}
BinaryArchiveWriter::BinaryArchiveWriter(Stream& stream, Endianess endianess) :
	BinaryArchive(false, endianess),
	m_stream(stream)
{
}
bool BinaryArchiveWriter::expectBlob(const void* expected, size_t size)
{
	m_stream.write(expected, size);
	return true;
}
void BinaryArchiveWriter::parseBlob(void* value, size_t size)
{
	m_stream.write(value, size);
}

BinaryReader::BinaryReader(Stream& stream, Endianess endianess) :
	m_stream(stream),
	m_endianess(endianess)
{
}

TextReader::TextReader(Stream& stream, Endianess endianess) :
	m_stream(stream),
	m_endianess(endianess)
{
}

}