#pragma once

#include <Aka/OS/Stream/Stream.h>
#include <Aka/OS/Endian.h>
#include <Aka/Core/Container/String.h>

namespace aka {

class BinaryArchive
{
public:
	BinaryArchive(Stream& stream, Endianess endianess = Endianess::Default);

	template <typename T> T read();
	template <typename T> void read(T& data);
	template <typename T> void read(T* data, size_t count);

	template <typename T> void write(const T& value);
	template <typename T> void write(const T* value, size_t count);

	void skim(size_t count) { m_stream.skim(count); }
	void seek(size_t position) { m_stream.seek(position); }
	size_t size() const { return m_stream.size(); }
	size_t offset() { return m_stream.offset(); }
	void rewind() { m_stream.rewind(); }
private:
	Stream& m_stream;
	Endianess m_endianess;
};

class TextArchive // support unicode
{
public:
	using Character = char;
public:
	TextArchive(Stream& stream, Endianess endianess = Endianess::Default);

	// Get the next line in the stream.
	bool readLine(Str<Character>& line);
	// Get the next word in the stream.
	bool readWord(Str<Character>& word);

	// Write a word
	void write(const Str<Character>& text);
	// Write a word
	void write(const Character* text, size_t length);

	void skim(size_t count) { m_stream.skim(count); }
	void seek(size_t position) { m_stream.seek(position); }
	size_t size() const { return m_stream.size(); }
	size_t offset() { return m_stream.offset(); }
	void rewind() { m_stream.rewind(); }
private:
	Stream& m_stream;
	Endianess m_endianess;
};


template<typename T>
T BinaryArchive::read()
{
	T data;
	read<T>(&data, 1);
	return data;
}

template<typename T>
void BinaryArchive::read(T& data)
{
	read<T>(&data, 1);
}

template<typename T>
void BinaryArchive::read(T* data, size_t count)
{
	//static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Do not support non arithmetic type");
	m_stream.unserialize(data, count * sizeof(T));
	if (!Endian::same(m_endianess))
	{
		for (size_t i = 0; i < count; i++)
		{
			Endian::swap<T>(data[i]);
		}
	}
}

template<typename T>
void BinaryArchive::write(const T& data)
{
	write<T>(&data, 1);
}

template<typename T>
void BinaryArchive::write(const T* data, size_t count)
{
	//static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Do not support non arithmetic type");
	if (!Endian::same(m_endianess))
	{
		for (size_t i = 0; i < count; i++)
		{
			T tmp = data[i];
			Endian::swap<T>(tmp);
			m_stream.serialize(&tmp, sizeof(T));
		}
	}
	else
	{
		m_stream.serialize(data, count * sizeof(T));
	}
}

};