#pragma once

#include <vector>

#include <Aka/Core/Debug.h>

namespace aka {

template <typename T>
class Vector final
{
public:
	Vector();
	Vector(const std::vector<T>& vector);
	Vector(T* data, size_t size);
	Vector(size_t size, T&& value);
	Vector(const Vector& vector);
	Vector(Vector&& vector);
	explicit Vector(size_t size);
	Vector& operator=(const Vector& vector);
	Vector& operator=(Vector&& vector);
	~Vector();

	operator std::vector<T>() const;

	T& operator[](size_t index);
	const T& operator[](size_t index) const;

	bool operator==(const Vector<T>& vector) const;
	bool operator!=(const Vector<T>& vector) const;
	bool operator<(const Vector<T>& vector) const;

	Vector<T>& append(const Vector<T>& vector);
	Vector<T>& append(const T* start, const T* end);
	Vector<T>& append(const T& value);
	Vector<T>& append(T&& value);
	Vector<T>& append();

	void remove(T* start, T* end);
	void remove(T* value);

	// Get the size of the vector
	size_t size() const;
	// Get the capacity of the vector
	size_t capacity() const;
	// Resize the vector
	void resize(size_t size);
	// Resize the vector capacity
	void reserve(size_t size);
	// Empty the vector
	void clear();
	// Check if vector empty
	bool empty() const;

	// Pointer to the vector
	T* data();
	// Pointer to the vector
	const T* data() const;
	// First element of vector
	T& first();
	// First element of vector
	const T& first() const;
	// Last element of vector
	T& last();
	// Last element of vector
	const T& last() const;

	// Pointer to beginning of vector
	T* begin();
	// Pointer to ending of vector
	T* end();
	// Pointer to beginning of vector
	const T* begin() const;
	// Pointer to ending of vector
	const T* end() const;
private:
	T* m_data;
	size_t m_size;
	size_t m_capacity;
};


template <typename T>
inline Vector<T>::Vector() :
	m_data(new T[16]),
	m_size(0),
	m_capacity(16)
{
}
template <typename T>
inline Vector<T>::Vector(const std::vector<T>& vector) :
	Vector(vector.data(), vector.size())
{
}
template <typename T>
inline Vector<T>::Vector(T* data, size_t size) :
	m_data(new T[size]),
	m_size(size),
	m_capacity(size)
{
	memcpy(m_data, data, sizeof(T) * size);
}
template <typename T>
inline Vector<T>::Vector(size_t size, T&& value) :
	m_data(new T[size]),
	m_size(size),
	m_capacity(size)
{
	for (T& data : m_data)
		data = std::move(value);
}
template <typename T>
inline Vector<T>::Vector(const Vector& vector) :
	Vector(vector.data(), vector.size())
{
}
template <typename T>
inline Vector<T>::Vector(Vector&& vector) :
	m_data(nullptr),
	m_size(0),
	m_capacity(0)
{
	std::swap(m_data, vector.m_data);
	std::swap(m_size, vector.m_size);
	std::swap(m_capacity, vector.m_capacity);
}
template <typename T>
inline Vector<T>::Vector(size_t size) :
	Vector(size, T())
{
}
template <typename T>
inline Vector<T>& Vector<T>::operator=(const Vector& vector)
{
	resize(vector.size());
	memcpy(m_data, vector.m_data, m_size * sizeof(T));
}
template <typename T>
inline Vector<T>& Vector<T>::operator=(Vector&& vector)
{
	std::swap(m_data, vector.m_data);
	std::swap(m_size, vector.m_size);
	std::swap(m_capacity, vector.m_capacity);
}
template <typename T>
inline Vector<T>::~Vector()
{
	delete[] m_data;
}
template <typename T>
inline Vector<T>::operator std::vector<T>() const
{
	return std::vector<T>(m_data, m_data + m_size);
}
template <typename T>
inline T& Vector<T>::operator[](size_t index)
{
	AKA_ASSERT(index < m_size, "Out of range");
	return m_data[index];
}
template <typename T>
inline const T& Vector<T>::operator[](size_t index) const
{
	AKA_ASSERT(index < m_size, "Out of range");
	return m_data[index];
}
template <typename T>
inline bool Vector<T>::operator==(const Vector<T>& value) const
{
}
template <typename T>
inline bool Vector<T>::operator!=(const Vector<T>& value) const
{
}
template <typename T>
inline bool Vector<T>::operator<(const Vector<T>& value) const
{
}
template <typename T>
inline Vector<T>& Vector<T>::append(const Vector<T>& vector)
{
	return append(vector.data(), vector.data() + vector.size());
}
template <typename T>
inline Vector<T>& Vector<T>::append(const T* start, const T*end)
{
	AKA_ASSERT(end >= start, "Invalid range");
	size_t off = m_size;
	resize(m_size + (end - start));
	memcpy(m_data + off, start, (end - start) * sizeof(T));
	return *this;
}
template <typename T>
inline Vector<T>& Vector<T>::append(const T& value)
{
	size_t off = m_size;
	resize(m_size + 1);
	m_data[off] = value;
	return *this;
}
template <typename T>
inline Vector<T>& Vector<T>::append(T&& value)
{
	size_t off = m_size;
	resize(m_size + 1);
	m_data[off] = std::move(value);
	return *this;
}
template <typename T>
inline void Vector<T>::remove(T* start, T* end)
{
	if (start < m_data && start >= m_data + m_size)
		throw std::invalid_argument("Start not in range");
	if (end <= m_data || end > m_data + m_size)
		throw std::invalid_argument("End not in range");
	if (end < start)
		throw std::invalid_argument("Invalid range");
	size_t range = m_data + m_size - end;
	memcpy(start, end, range * sizeof(T));
	m_size -= (end - start);
}
template <typename T>
inline void Vector<T>::remove(T* value)
{
	remove(value, value + 1);
}
template <typename T>
inline size_t Vector<T>::size() const
{
	return m_size;
}
template <typename T>
inline size_t Vector<T>::capacity() const
{
	return m_capacity;
}
template <typename T>
inline void Vector<T>::resize(size_t size)
{
	if (m_size == size)
		return;
	reserve(size);
	m_size = size;
}
template <typename T>
inline void Vector<T>::reserve(size_t size)
{
	if (size <= m_capacity)
		return;
	size_t oldCapacity = m_capacity;
	while (m_capacity < size)
		m_capacity *= 2;
	T* buffer = new T[m_capacity]();
	memcpy(buffer, m_data, oldCapacity);
	delete[] m_data;
	m_data = buffer;
}
template <typename T>
inline void Vector<T>::clear()
{
	m_size = 0;
}
template <typename T>
inline bool Vector<T>::empty() const
{
	return m_size == 0;
}
template <typename T>
inline T* Vector<T>::data()
{
	return m_data;
}
template <typename T>
inline const T* Vector<T>::data() const
{
	return m_data;
}
template <typename T>
inline T& Vector<T>::first()
{
	return m_data[0];
}
template <typename T>
inline const T& Vector<T>::first() const
{
	return m_data[0];
}
template <typename T>
inline T& Vector<T>::last()
{
	return m_data[m_size - 1];
}
template <typename T>
inline const T& Vector<T>::last() const
{
	return m_data[m_size - 1];
}
template <typename T>
inline T* Vector<T>::begin()
{
	return m_data;
}
template <typename T>
inline T* Vector<T>::end()
{
	return m_data + m_size;
}
template <typename T>
inline const T* Vector<T>::begin() const
{
	return m_data;
}
template <typename T>
inline const T* Vector<T>::end() const
{
	return m_data + m_size;
}

};
