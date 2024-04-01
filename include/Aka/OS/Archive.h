#pragma once

#include <Aka/OS/Stream/Stream.h>
#include <Aka/OS/Endian.h>
#include <Aka/Core/Container/String.h>

#include <functional>

namespace aka {

class BinaryReader
{
public:
	BinaryReader(Stream& stream, Endianess endianess = Endianess::Default);

	template <typename T> T read();
	template <typename T> void read(T& data);
	template <typename T> void read(T* data, size_t count);

	void skim(size_t count) { m_stream.skim(count); }
	void seek(size_t position) { m_stream.seek(position); }
	size_t size() const { return m_stream.size(); }
	size_t offset() { return m_stream.offset(); }
	void rewind() { m_stream.rewind(); }
private:
	Stream& m_stream;
	Endianess m_endianess;
};

class TextReader // support unicode
{
public:
	using Character = char;
public:
	TextReader(Stream& stream, Endianess endianess = Endianess::Default);

	// Get the next line in the stream.
	bool readLine(Str<Character>& line);
	// Get the next word in the stream.
	bool readWord(Str<Character>& word);

	// Write a word
	//void write(const Str<Character>& text);
	// Write a word
	//void write(const Character* text, size_t length);

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
T BinaryReader::read()
{
	T data;
	read<T>(&data, 1);
	return data;
}

template<typename T>
void BinaryReader::read(T& data)
{
	read<T>(&data, 1);
}

template<typename T>
void BinaryReader::read(T* data, size_t count)
{
	static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Do not support non arithmetic type");
	m_stream.read(data, count * sizeof(T));
	if (!Endian::same(m_endianess))
	{
		for (size_t i = 0; i < count; i++)
		{
			Endian::swap<T>(data[i]);
		}
	}
}

class BinaryArchive
{
public:
	BinaryArchive(bool isReading, Endianess endianess = Endianess::Default);

	template <typename T>
	bool expect(const T& expected);
	template <typename T>
	void parse(T& value);
	template <typename T>
	void parse(Vector<T>& value);
	void parse(String& value);
	void parse(point3f& value);
	void parse(vec3f& value);
	void parse(quatf& value);
	void parse(color4f& value);
	void parse(mat4f& value);
	template <typename T>
	void parse(Vector<T>& value, std::function<void(BinaryArchive&, T&)>&& callback);

	virtual bool expectBlob(const void* value, size_t size) = 0;
	virtual void parseBlob(void* value, size_t size) = 0;

	bool isReading() const { return m_reading; }
	bool isWriting() const { return !m_reading; }
protected:
	Vector<byte_t> m_cache;
	Endianess m_endianess;
	bool m_reading; // If not reading, writing
};

struct BinaryArchiveReader : BinaryArchive
{
	BinaryArchiveReader(Stream& stream, Endianess endianess = Endianess::Default);
	bool expectBlob(const void* expected, size_t size) override;
	void parseBlob(void* value, size_t size) override;
private:
	Stream& m_stream;
};

struct BinaryArchiveWriter : BinaryArchive
{
	BinaryArchiveWriter(Stream& stream, Endianess endianess = Endianess::Default);
	bool expectBlob(const void* expected, size_t size) override;
	void parseBlob(void* value, size_t size) override;
private:
	Stream& m_stream;
};


template <typename T>
bool BinaryArchive::expect(const T& expected)
{
	static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Do not support non arithmetic type");
	return expectBlob(&expected, sizeof(T));
}
template <typename T>
void BinaryArchive::parse(T& value)
{
	static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Do not support non arithmetic type");
	if (isWriting() && !Endian::same(m_endianess))
		Endian::swap<T>(value);
	parseBlob(&value, sizeof(T));
	if (isReading() && !Endian::same(m_endianess))
		Endian::swap<T>(value);
}


template <typename T>
void BinaryArchive::parse(Vector<T>& value)
{
	static_assert(std::is_trivially_copyable<T>::value, "Do not support non trivially copyable type. use lambda variant to define how to copy content.");
	uint32_t size = static_cast<uint32_t>(value.size());
	parse<uint32_t>(size);
	if (isReading())
		value.resize(size);
	parseBlob(value.data(), value.size() * sizeof(T));
}


template <typename T>
void BinaryArchive::parse(Vector<T>& value, std::function<void(BinaryArchive&, T&)>&& callback)
{
	uint32_t size = static_cast<uint32_t>(value.size());
	parse<uint32_t>(size);
	if (isReading())
		value.resize(size);
	for (T& element : value)
		callback(*this, element);
}

};