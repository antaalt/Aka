#pragma once

#include <memory>

#include <Aka/Core/Config.h>
#include <Aka/Memory/Memory.h>

namespace aka {

template <typename T, class AllocatorType = mem::DefaultAllocatorType>
class Vector final
{
	static const size_t defaultCapacity = 16;
public:
	Vector();
	explicit Vector(AllocatorType& allocator);
	explicit Vector(const T* data, size_t size, AllocatorType& allocator = mem::DefaultAllocator);
	explicit Vector(size_t size, const T& defaultValue, AllocatorType& allocator = mem::DefaultAllocator);
	explicit Vector(size_t size, AllocatorType& allocator = mem::DefaultAllocator);
	Vector(const Vector& vector, AllocatorType& allocator = mem::DefaultAllocator);
	Vector(Vector&& vector);
	Vector& operator=(const Vector& vector);
	Vector& operator=(Vector&& vector);
	~Vector();

	T& operator[](size_t index);
	const T& operator[](size_t index) const;

	bool operator==(const Vector<T, AllocatorType>& vector) const;
	bool operator!=(const Vector<T, AllocatorType>& vector) const;
	bool operator<(const Vector<T, AllocatorType>& vector) const;

	Vector<T, AllocatorType>& append(const Vector<T, AllocatorType>& vector);
	Vector<T, AllocatorType>& append(const T* start, const T* end);
	Vector<T, AllocatorType>& append(const T& value);
	Vector<T, AllocatorType>& append(T&& value);

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
	AllocatorType& m_allocator;
	T* m_data;
	size_t m_size;
	size_t m_capacity;
};

template <typename T, typename AllocatorType>
inline Vector<T, AllocatorType>::Vector() :
	Vector(mem::DefaultAllocator)
{
}
template<typename T, class AllocatorType>
inline Vector<T, AllocatorType>::Vector(AllocatorType& allocator) :
	m_allocator(mem::DefaultAllocator),
	m_data(static_cast<T*>(m_allocator.allocate(defaultCapacity * sizeof(T)))),
	m_size(0),
	m_capacity(defaultCapacity)
{
}
template <typename T, typename AllocatorType>
inline Vector<T, AllocatorType>::Vector(const T* data, size_t size, AllocatorType& allocator) :
	m_allocator(allocator),
	m_data(static_cast<T*>(m_allocator.allocate(size * sizeof(T)))),
	m_size(size),
	m_capacity(size)
{
	std::uninitialized_copy(data, data + size, begin());
}
template <typename T, typename AllocatorType>
inline Vector<T, AllocatorType>::Vector(size_t size, const T& value, AllocatorType& allocator) :
	m_allocator(allocator),
	m_data(static_cast<T*>(m_allocator.allocate(size * sizeof(T)))),
	m_size(size),
	m_capacity(size)
{
	std::uninitialized_fill(begin(), end(), value);
}
template <typename T, typename AllocatorType>
inline Vector<T, AllocatorType>::Vector(size_t size, AllocatorType& allocator) :
	m_allocator(allocator),
	m_data(static_cast<T*>(m_allocator.allocate(size * sizeof(T)))),
	m_size(size),
	m_capacity(size)
{
	std::uninitialized_default_construct(begin(), end());
}
template <typename T, typename AllocatorType>
inline Vector<T, AllocatorType>::Vector(const Vector& vector, AllocatorType& allocator) :
	Vector(vector.data(), vector.size(), allocator)
{
}
template <typename T, typename AllocatorType>
inline Vector<T, AllocatorType>::Vector(Vector&& vector) :
	m_allocator(mem::DefaultAllocator),
	m_data(nullptr),
	m_size(0),
	m_capacity(0)
{
	//std::swap(m_allocator, vector.m_allocator);
	std::swap(m_data, vector.m_data);
	std::swap(m_size, vector.m_size);
	std::swap(m_capacity, vector.m_capacity);
}
template <typename T, typename AllocatorType>
inline Vector<T, AllocatorType>& Vector<T, AllocatorType>::operator=(const Vector& vector)
{
	resize(vector.size());
	std::copy(vector.data(), vector.data() + vector.size(), begin());
	return *this;
}
template <typename T, typename AllocatorType>
inline Vector<T, AllocatorType>& Vector<T, AllocatorType>::operator=(Vector&& vector)
{
	//std::swap(m_allocator, vector.m_allocator);
	std::swap(m_data, vector.m_data);
	std::swap(m_size, vector.m_size);
	std::swap(m_capacity, vector.m_capacity);
	return *this;
}
template <typename T, typename AllocatorType>
inline Vector<T, AllocatorType>::~Vector()
{
	if (m_capacity > 0)
	{
		std::destroy(begin(), end());
		m_allocator.deallocate(m_data, m_capacity * sizeof(T));
	}
}
template <typename T, typename AllocatorType>
inline T& Vector<T, AllocatorType>::operator[](size_t index)
{
	AKA_ASSERT(index < m_size, "Out of range");
	return m_data[index];
}
template <typename T, typename AllocatorType>
inline const T& Vector<T, AllocatorType>::operator[](size_t index) const
{
	AKA_ASSERT(index < m_size, "Out of range");
	return m_data[index];
}
template <typename T, typename AllocatorType>
inline bool Vector<T, AllocatorType>::operator==(const Vector<T, AllocatorType>& value) const
{
	if (size() != value.size())
		return false;
	// TODO: use std::equal
	for (size_t i = 0; i < size(); i++)
	{
		if (m_data[i] != value.m_data[i])
			return false;
	}
	return true;
}
template <typename T, typename AllocatorType>
inline bool Vector<T, AllocatorType>::operator!=(const Vector<T, AllocatorType>& value) const
{
	if (size() != value.size())
		return true;
	for (size_t i = 0; i < size(); i++)
	{
		if (m_data[i] != value.m_data[i])
			return true;
	}
	return false;
}
template <typename T, typename AllocatorType>
inline bool Vector<T, AllocatorType>::operator<(const Vector<T, AllocatorType>& value) const
{
	AKA_NOT_IMPLEMENTED;
}
template <typename T, typename AllocatorType>
inline Vector<T, AllocatorType>& Vector<T, AllocatorType>::append(const Vector<T, AllocatorType>& vector)
{
	return append(vector.data(), vector.data() + vector.size());
}
template <typename T, typename AllocatorType>
inline Vector<T, AllocatorType>& Vector<T, AllocatorType>::append(const T* _start, const T* _end)
{
	AKA_ASSERT(_end >= _start, "Invalid range");
	T* b = begin();
	T* e = end();
	size_t size = m_size;
	size_t range = (_end - _start);
	resize(m_size + range);
	std::uninitialized_copy(_start, _end, m_data + size);
	return *this;
}
template <typename T, typename AllocatorType>
inline Vector<T, AllocatorType>& Vector<T, AllocatorType>::append(const T& value)
{
	size_t off = m_size;
	resize(m_size + 1);
	std::uninitialized_copy_n(&value, 1, m_data + off);
	return *this;
}
template <typename T, typename AllocatorType>
inline Vector<T, AllocatorType>& Vector<T, AllocatorType>::append(T&& value)
{
	size_t off = m_size;
	resize(m_size + 1);
	std::uninitialized_move_n(&value, 1, m_data + off);
	return *this;
}
template <typename T, typename AllocatorType>
inline void Vector<T, AllocatorType>::remove(T* start, T* end)
{
	AKA_ASSERT(start >= m_data || start < m_data + m_size, "Start not in range");
	AKA_ASSERT(end > m_data || end <= m_data + m_size, "End not in range");
	AKA_ASSERT(end < start, "Invalid range");
	size_t range = m_data + m_size - end;
	std::copy(end, m_data + m_size, start);
	std::destroy(end, m_data + m_size);
	m_size -= (end - start);
}
template <typename T, typename AllocatorType>
inline void Vector<T, AllocatorType>::remove(T* value)
{
	remove(value, value + 1);
}
template <typename T, typename AllocatorType>
inline size_t Vector<T, AllocatorType>::size() const
{
	return m_size;
}
template <typename T, typename AllocatorType>
inline size_t Vector<T, AllocatorType>::capacity() const
{
	return m_capacity;
}
template <typename T, typename AllocatorType>
inline void Vector<T, AllocatorType>::resize(size_t size)
{
	if (m_size == size)
		return;
	reserve(size);
	m_size = size;
}
template <typename T, typename AllocatorType>
inline void Vector<T, AllocatorType>::reserve(size_t size)
{
	if (size <= m_capacity)
		return;
	T* b = begin();
	T* e = end();
	size_t oldCapacity = m_capacity;
	m_capacity = size;
	T* buffer = static_cast<T*>(m_allocator.allocate(m_capacity * sizeof(T)));
	std::uninitialized_move(b, e, buffer);
	std::destroy(b, e); // needed ?
	m_allocator.deallocate(m_data, oldCapacity * sizeof(T));
	m_data = buffer;
}
template <typename T, typename AllocatorType>
inline void Vector<T, AllocatorType>::clear()
{
	m_size = 0;
}
template <typename T, typename AllocatorType>
inline bool Vector<T, AllocatorType>::empty() const
{
	return m_size == 0;
}
template <typename T, typename AllocatorType>
inline T* Vector<T, AllocatorType>::data()
{
	return m_data;
}
template <typename T, typename AllocatorType>
inline const T* Vector<T, AllocatorType>::data() const
{
	return m_data;
}
template <typename T, typename AllocatorType>
inline T& Vector<T, AllocatorType>::first()
{
	return m_data[0];
}
template <typename T, typename AllocatorType>
inline const T& Vector<T, AllocatorType>::first() const
{
	return m_data[0];
}
template <typename T, typename AllocatorType>
inline T& Vector<T, AllocatorType>::last()
{
	return m_data[m_size - 1];
}
template <typename T, typename AllocatorType>
inline const T& Vector<T, AllocatorType>::last() const
{
	return m_data[m_size - 1];
}
template <typename T, typename AllocatorType>
inline T* Vector<T, AllocatorType>::begin()
{
	return m_data;
}
template <typename T, typename AllocatorType>
inline T* Vector<T, AllocatorType>::end()
{
	return m_data + m_size;
}
template <typename T, typename AllocatorType>
inline const T* Vector<T, AllocatorType>::begin() const
{
	return m_data;
}
template <typename T, typename AllocatorType>
inline const T* Vector<T, AllocatorType>::end() const
{
	return m_data + m_size;
}

};
