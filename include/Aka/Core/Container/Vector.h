#pragma once

#include <memory>

#include <Aka/Core/Config.h>
#include <Aka/Memory/Memory.h>

namespace aka {

template <typename T>
class Vector final
{
	using AllocatorType = Allocator;
	static const size_t defaultCapacity = 16;
public:
	Vector();
	explicit Vector(AllocatorType& allocator);
	explicit Vector(const T* data, size_t size, AllocatorType& allocator = mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default));
	explicit Vector(size_t size, const T& defaultValue, AllocatorType& allocator = mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default));
	explicit Vector(size_t size, AllocatorType& allocator = mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default));
	Vector(const Vector& vector, AllocatorType& allocator = mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default));
	Vector(Vector&& vector);
	Vector& operator=(const Vector& vector);
	Vector& operator=(Vector&& vector);
	~Vector();

	T& operator[](size_t index);
	const T& operator[](size_t index) const;

	bool operator==(const Vector<T>& vector) const;
	bool operator!=(const Vector<T>& vector) const;
	bool operator<(const Vector<T>& vector) const;

	Vector<T>& append(const Vector<T>& vector);
	Vector<T>& append(const T* start, const T* end);
	Vector<T>& append(const T& value);
	Vector<T>& append(T&& value);

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

template <typename T>
inline Vector<T>::Vector() :
	Vector(mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default))
{
}
template<typename T>
inline Vector<T>::Vector(AllocatorType& allocator) :
	m_allocator(mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default)),
	m_data(static_cast<T*>(m_allocator.allocate(defaultCapacity * sizeof(T)))),
	m_size(0),
	m_capacity(defaultCapacity)
{
}
template <typename T>
inline Vector<T>::Vector(const T* data, size_t size, AllocatorType& allocator) :
	m_allocator(allocator),
	m_data(static_cast<T*>(m_allocator.allocate(size * sizeof(T)))),
	m_size(size),
	m_capacity(size)
{
	std::uninitialized_copy(data, data + size, begin());
}
template <typename T>
inline Vector<T>::Vector(size_t size, const T& value, AllocatorType& allocator) :
	m_allocator(allocator),
	m_data(static_cast<T*>(m_allocator.allocate(size * sizeof(T)))),
	m_size(size),
	m_capacity(size)
{
	std::uninitialized_fill(begin(), end(), value);
}
template <typename T>
inline Vector<T>::Vector(size_t size, AllocatorType& allocator) :
	m_allocator(allocator),
	m_data(static_cast<T*>(m_allocator.allocate(size * sizeof(T)))),
	m_size(size),
	m_capacity(size)
{
	std::uninitialized_default_construct(begin(), end());
}
template <typename T>
inline Vector<T>::Vector(const Vector& vector, AllocatorType& allocator) :
	Vector(vector.data(), vector.size(), allocator)
{
}
template <typename T>
inline Vector<T>::Vector(Vector&& vector) :
	m_allocator(mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default)),
	m_data(nullptr),
	m_size(0),
	m_capacity(0)
{
	//std::swap(m_allocator, vector.m_allocator);
	std::swap(m_data, vector.m_data);
	std::swap(m_size, vector.m_size);
	std::swap(m_capacity, vector.m_capacity);
}
template <typename T>
inline Vector<T>& Vector<T>::operator=(const Vector& vector)
{
	resize(vector.size());
	std::copy(vector.data(), vector.data() + vector.size(), begin());
	return *this;
}
template <typename T>
inline Vector<T>& Vector<T>::operator=(Vector&& vector)
{
	//std::swap(m_allocator, vector.m_allocator);
	std::swap(m_data, vector.m_data);
	std::swap(m_size, vector.m_size);
	std::swap(m_capacity, vector.m_capacity);
	return *this;
}
template <typename T>
inline Vector<T>::~Vector()
{
	if (m_capacity > 0)
	{
		std::destroy(begin(), end());
		m_allocator.deallocate(m_data, m_capacity * sizeof(T));
	}
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
template <typename T>
inline bool Vector<T>::operator!=(const Vector<T>& value) const
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
template <typename T>
inline bool Vector<T>::operator<(const Vector<T>& value) const
{
	AKA_NOT_IMPLEMENTED;
}
template <typename T>
inline Vector<T>& Vector<T>::append(const Vector<T>& vector)
{
	return append(vector.data(), vector.data() + vector.size());
}
template <typename T>
inline Vector<T>& Vector<T>::append(const T* _start, const T* _end)
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
template <typename T>
inline Vector<T>& Vector<T>::append(const T& value)
{
	size_t off = m_size;
	resize(m_size + 1);
	std::uninitialized_copy_n(&value, 1, m_data + off);
	return *this;
}
template <typename T>
inline Vector<T>& Vector<T>::append(T&& value)
{
	size_t off = m_size;
	resize(m_size + 1);
	std::uninitialized_move_n(&value, 1, m_data + off);
	return *this;
}
template <typename T>
inline void Vector<T>::remove(T* start, T* end)
{
	AKA_ASSERT(start >= m_data || start < m_data + m_size, "Start not in range");
	AKA_ASSERT(end > m_data || end <= m_data + m_size, "End not in range");
	AKA_ASSERT(end < start, "Invalid range");
	size_t range = m_data + m_size - end;
	std::copy(end, m_data + m_size, start);
	std::destroy(end, m_data + m_size);
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
