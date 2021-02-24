#pragma once

#include <stdint.h>
#include <vector>
#include <stdexcept>

#include <Aka/Core/Debug.h>
#include <Aka/OS/Endian.h>
#include <Aka/Platform/Platform.h>

namespace aka {

class Stream;

// Get a line of text from the stream
std::string readLine(Stream& stream);
// Read a string from a stream until a null terminated character is met
std::string readString(Stream& stream);

class Stream
{
private:
	template< typename T >
	using IsArithmetic = typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, bool>::type;
public:
	Stream() {}
	Stream(const Stream&) = delete;
	Stream& operator=(const Stream&) = delete;
	virtual ~Stream() {}

	// Read a single element at cursor and move it. Take care of endianess.
	template <typename T, IsArithmetic<T> = true> T read(Endianess::Order order = Endianess::Order::Little);
	// Read multiple elements at cursor and move it. Take care of endianess.
	template <typename T, IsArithmetic<T> = true> void read(T* data, size_t count, Endianess::Order order = Endianess::Order::Little);
	// Read multiple elements at cursor and move it.
	void read(void* data, size_t size) { readData(data, size); }
	// Write a single element at cursor and move it. Take care of endianess.
	template <typename T, IsArithmetic<T> = true> void write(const T& element, Endianess::Order order = Endianess::Order::Little);
	// Write multiple elements at cursor and move it. Take care of endianess.
	template <typename T, IsArithmetic<T> = true> void write(const T* element, size_t count, Endianess::Order order = Endianess::Order::Little);
	// Write multiple elements at cursor and move it.
	void write(const void* data, size_t size) { writeData(data, size); }

	// Simply forward the cursor.
	virtual void skim(size_t count) = 0;
	// Seek the cursor
	virtual void seek(size_t position) = 0;
	// Get the size of the whole buffer.
	virtual size_t size() const = 0;
	// Get the position of the cursor.
	virtual size_t offset() = 0;
	// Set the cursor position at 0.
	virtual void rewind() = 0;
protected:
	// Read data logic to be implemented in child class
	virtual void readData(void* data, size_t size) = 0;
	// Write data logic to be implemented in child class
	virtual void writeData(const void* data, size_t size) = 0;
};

template <typename T, Stream::IsArithmetic<T>>
T Stream::read(Endianess::Order order)
{
	T element;
	read(&element, 1, order);
	return element;
}

template <typename T, Stream::IsArithmetic<T>>
void Stream::read(T* data, size_t count, Endianess::Order order)
{
	readData(data, count * sizeof(T));
	if (!Endianess::same(order))
		for(size_t i = 0; i < count; i++)
			Endianess::swap<T>(data[i]);
}

template <typename T, Stream::IsArithmetic<T>>
void Stream::write(const T& element, Endianess::Order order)
{
	write(&element, 1, order);
}

template <typename T, Stream::IsArithmetic<T>>
void Stream::write(const T* elements, size_t count, Endianess::Order order)
{

	if (!Endianess::same(order))
	{
		for (size_t i = 0; i < count; i++)
		{
			T element = elements[i];
			Endianess::swap<T>(element);
			write(&element, 1);
		}
	}
	else
	{
		writeData(elements, count * sizeof(T));
	}
}

};