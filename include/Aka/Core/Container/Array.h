#pragma once

#include <Aka/Core/Config.h>

namespace aka {

template <typename T, size_t Size>
class Array final
{
public:
	T& operator[](size_t index);
	const T& operator[](size_t index) const;

	// Get the size of the array
	size_t size() const;

	// Pointer to the array
	T* data();
	// Pointer to the array
	const T* data() const;
	// First element of array
	T& first();
	// First element of array
	const T& first() const;
	// Last element of array
	T& last();
	// Last element of array
	const T& last() const;

	// Pointer to beginning of array
	T* begin();
	// Pointer to ending of array
	T* end();
	// Pointer to beginning of array
	const T* begin() const;
	// Pointer to ending of array
	const T* end() const;
private:
	T m_data[Size];
};

template <typename T, size_t Size>
inline T& Array<T, Size>::operator[](size_t index)
{
	return m_data[index];
}
template <typename T, size_t Size>
inline const T& Array<T, Size>::operator[](size_t index) const
{
	return m_data[index];
}
template <typename T, size_t Size>
inline size_t Array<T, Size>::size() const
{
	return Size;
}
template <typename T, size_t Size>
inline T* Array<T, Size>::data()
{
	return m_data;
}
template <typename T, size_t Size>
inline const T* Array<T, Size>::data() const
{
	return m_data;
}
template <typename T, size_t Size>
inline T& Array<T, Size>::first()
{
	return m_data[0];
}
template <typename T, size_t Size>
inline const T& Array<T, Size>::first() const
{
	return m_data[0];
}
template <typename T, size_t Size>
inline T& Array<T, Size>::last()
{
	return m_data[Size - 1];
}
template <typename T, size_t Size>
inline const T& Array<T, Size>::last() const
{
	return m_data[Size - 1];
}
template <typename T, size_t Size>
inline T* Array<T, Size>::begin()
{
	return m_data;
}
template <typename T, size_t Size>
inline T* Array<T, Size>::end()
{
	return m_data + Size;
}
template <typename T, size_t Size>
inline const T* Array<T, Size>::begin() const
{
	return m_data;
}
template <typename T, size_t Size>
inline const T* Array<T, Size>::end() const
{
	return m_data + Size;
}

};
