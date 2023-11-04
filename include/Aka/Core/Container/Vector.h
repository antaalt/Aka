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
	using Iterator = T*; // TODO use real iterator instead.
	using ConstIterator = const T*;
public:
	Vector();
	explicit Vector(AllocatorType& allocator);
	explicit Vector(const T* data, size_t size, AllocatorType& allocator = mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default));
	explicit Vector(size_t size, const T& defaultValue, AllocatorType& allocator = mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default));
	explicit Vector(size_t size, AllocatorType& allocator = mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default));
	Vector(const Vector& vector, AllocatorType& allocator = mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default));
	Vector(Vector&& vector, AllocatorType& allocator = mem::getAllocator(mem::AllocatorMemoryType::Persistent, mem::AllocatorCategory::Default));
	Vector& operator=(const Vector& vector);
	Vector& operator=(Vector&& vector);
	~Vector();

	T& operator[](size_t index);
	const T& operator[](size_t index) const;

	bool operator==(const Vector<T>& vector) const;
	bool operator!=(const Vector<T>& vector) const;
	bool operator<(const Vector<T>& vector) const;
	bool operator>(const Vector<T>& vector) const;
	bool operator<=(const Vector<T>& vector) const;
	bool operator>=(const Vector<T>& vector) const;

	T& append(const Vector<T>& vector);
	T& append(const T* start, const T* end);
	T& append(const T& value);
	T& append(T&& value);
	template <typename ...Args>
	T& emplace(Args ...args);

	void remove(T* start, T* end);
	void remove(T* value);

	// Get the size of the vector
	size_t size() const;
	// Get the capacity of the vector
	size_t capacity() const;
	// Resize the vector
	void resize(size_t size);
	// Resize the vector with default value
	void resize(size_t size, const T& defaultValue);
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
	Iterator begin();
	// Pointer to ending of vector
	Iterator end();
	// Pointer to beginning of vector
	ConstIterator begin() const;
	// Pointer to ending of vector
	ConstIterator end() const;
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
inline Vector<T>::Vector(Vector&& vector, AllocatorType& allocator) :
	m_allocator(allocator),
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
	return (size() == value.size()) && std::equal(begin(), end(), value.begin());
}
template <typename T>
inline bool Vector<T>::operator!=(const Vector<T>& value) const
{
	return (size() != value.size()) || !std::equal(begin(), end(), value.begin());
}
template <typename T>
inline bool Vector<T>::operator<(const Vector<T>& value) const
{
	return std::lexicographical_compare(begin(), end(), value.begin(), value.end());
}
template <typename T>
inline bool Vector<T>::operator>(const Vector<T>& value) const
{
	return value < *this;
}
template <typename T>
inline bool Vector<T>::operator<=(const Vector<T>& value) const
{
	return !(value < *this);
}
template <typename T>
inline bool Vector<T>::operator>=(const Vector<T>& value) const
{
	return !(*this < value);
}
template <typename T>
inline T& Vector<T>::append(const Vector<T>& vector)
{
	return append(vector.data(), vector.data() + vector.size());
}
template <typename T>
inline T& Vector<T>::append(const T* _start, const T* _end)
{
	AKA_ASSERT(_end >= _start, "Invalid range");
	T* b = begin();
	T* e = end();
	size_t size = m_size;
	size_t range = (_end - _start);
	resize(m_size + range);
	std::uninitialized_copy(_start, _end, m_data + size);
	return last();
}
template <typename T>
inline T& Vector<T>::append(const T& value)
{
	size_t off = m_size;
	resize(m_size + 1);
	new (m_data + off) T(value);
	return last();
}
template <typename T>
inline T& Vector<T>::append(T&& value)
{
	size_t off = m_size;
	resize(m_size + 1);
	new (m_data + off) T(std::move(value));
	return last();
}
template<typename T>
template<typename ...Args>
inline T& Vector<T>::emplace(Args ...args)
{
	size_t off = m_size;
	resize(m_size + 1);
	new (m_data + off) T(std::forward<Args>(args)...);
	return last();
}
template <typename T>
inline void Vector<T>::remove(T* _start, T* _end)
{
	AKA_ASSERT(_start >= m_data || _start < m_data + m_size, "Start not in range");
	AKA_ASSERT(_end > m_data || _end <= m_data + m_size, "End not in range");
	AKA_ASSERT(_end >= _start, "Invalid range");
	std::copy(_end, end(), _start);
	std::destroy(_end, end());
	m_size -= (_end - _start);
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
	if (m_size < size)
	{
		std::uninitialized_default_construct(m_data + m_size, m_data + size);
	}
	else
	{
		std::destroy(m_data + size, m_data + m_size);
	}
	m_size = size;
}
template <typename T>
inline void Vector<T>::resize(size_t size, const T& defaultValue)
{
	if (m_size == size)
		return;
	reserve(size);
	if (m_size < size)
	{
		std::uninitialized_fill(m_data + m_size, m_data + size, defaultValue);
	}
	else
	{
		std::destroy(m_data + size, m_data + m_size);
	}
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
