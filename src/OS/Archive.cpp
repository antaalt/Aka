#include <Aka/OS/Archive.h>

namespace aka {

BinaryArchive::BinaryArchive(Stream& stream, Endianess endianess) :
	m_stream(stream),
	m_endianess(endianess)
{
}

TextArchive::TextArchive(Stream& stream, Endianess endianess) :
	m_stream(stream),
	m_endianess(endianess)
{
}

}