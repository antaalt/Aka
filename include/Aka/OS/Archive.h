#pragma once

#include <Aka/OS/Stream/Stream.h>
#include <Aka/OS/Endian.h>
#include <Aka/Core/Container/String.h>

#include <functional>

namespace aka {

/*class BinaryArchive
{
public:
	template <typename T>
	bool expect(const T& expected); // ISPOD
	template <typename T>
	void parse(T& value); // ISPOD
	template <typename T>
	void parse(Vector<T>& value);
	void parse(String& value);
	template <typename T>
	void parse(Vector<T>& value, std::function<void(BinaryArchive& archive, T& value)>&& callback);


	virtual bool expectBlob(const void* value, size_t size) = 0;
	virtual void parseBlob(void* value, size_t size) = 0;

	bool isReading() const { return m_reading; }
	bool isWriting() const { return !m_reading; }
protected:
	Stream& m_stream;
	Endianess m_endianess;
	bool m_reading; // If not reading, writing
};

struct InputBinaryArchive : BinaryArchive
{
	Stream& a;
	bool expectBlob(const void* expected, size_t size)
	{
		Vector<byte_t> value(size);
		a.unserialize(value.data(), size);
		return Memory::compare(value.data(), expected, size);
	}
	void parseBlob(void* value, size_t size) override
	{
		a.unserialize(value, size);
	}
};

struct OutputBinaryArchive : BinaryArchive
{
	bool expect(const void* expected, size_t size)
	{
		a.serialize(expected, size);
		return true;
	}
	void parse(void* value, size_t size) override
	{
		a.serialize(value, size);
	}
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
		Endian::swap<T>(data[i]);
	parseBlob(&value, sizeof(T));
	if (isReading() && !Endian::same(m_endianess))
		Endian::swap<T>(data[i]);
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

template <typename T>
void BinaryArchive::parse(Vector<T>& value)
{
	static_assert(std::is_pod<T>::value, "Do not support non pod type");
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
	if (!m_reading)
		value.resize(size);
	for (T& element : value)
		callback(*this, element);
}
struct ToParse
{
	uint32_t data;
	Vector<ToParse> childrens;
};
Vector<ToParse> data;
void test()
{
	BinaryArchive archive;
	archive.expect("Yoho");
	archive.parse(data, [](BinaryArchive& archive, ToParse& value) {
		archive.parse(value.data);
		archive.parse(value.childrens); // This will break, non pod.
	});
}

*/







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