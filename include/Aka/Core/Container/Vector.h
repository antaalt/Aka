#pragma once

#include <memory>

#include <Aka/Core/Config.h>
#include <Aka/Memory/Memory.h>
#include <Aka/Core/Geometry.h>
#include <Aka/Core/Container/StlAllocator.hpp>

namespace aka {

// Override default vector for memory tracking
template <typename T, AllocatorCategory Category = AllocatorCategory::Global>
using vector = ::std::vector<T, AkaStlAllocator<T, AllocatorMemoryType::Vector, Category>>;

template <typename T, AllocatorCategory Category = AllocatorCategory::Global>
class Vector final
{
	static const size_t defaultCapacity = 16;
public:
	Vector();
	explicit Vector(Allocator& allocator);
	explicit Vector(const T* data, size_t size, Allocator& allocator = mem::getAllocator(AllocatorMemoryType::Vector, Category));
	explicit Vector(size_t size, const T& defaultValue, Allocator& allocator = mem::getAllocator(AllocatorMemoryType::Vector, Category));
	explicit Vector(size_t size, Allocator& allocator = mem::getAllocator(AllocatorMemoryType::Vector, Category));
	Vector(const Vector& vector);
	Vector(Vector&& vector);
	Vector& operator=(const Vector& vector);
	Vector& operator=(Vector&& vector);
	~Vector();

	T& operator[](size_t index);
	const T& operator[](size_t index) const;

	bool operator==(const Vector<T, Category>& vector) const;
	bool operator!=(const Vector<T, Category>& vector) const;
	bool operator<(const Vector<T, Category>& vector) const;
	bool operator>(const Vector<T, Category>& vector) const;
	bool operator<=(const Vector<T, Category>& vector) const;
	bool operator>=(const Vector<T, Category>& vector) const;

	T& append(const Vector<T, Category>& vector);
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
	T* begin();
	// Pointer to ending of vector
	T* end();
	// Pointer to beginning of vector
	const T* begin() const;
	// Pointer to ending of vector
	const T* end() const;
private:
	Allocator& m_allocator;
	T* m_data;
	size_t m_size;
	size_t m_capacity;
};

template <typename T, AllocatorCategory Category>
inline Vector<T, Category>::Vector() :
	Vector(mem::getAllocator(AllocatorMemoryType::Vector, Category))
{
}
template<typename T, AllocatorCategory Category>
inline Vector<T, Category>::Vector(Allocator& allocator) :
	m_allocator(allocator),
	m_data(m_allocator.allocate<T>(defaultCapacity)),
	m_size(0),
	m_capacity(defaultCapacity)
{
}
template <typename T, AllocatorCategory Category>
inline Vector<T, Category>::Vector(const T* data, size_t size, Allocator& allocator) :
	m_allocator(allocator),
	m_data(m_allocator.allocate<T>(max(size, defaultCapacity))),
	m_size(size),
	m_capacity(max(size, defaultCapacity))
{
	std::uninitialized_copy(data, data + size, begin());
}
template <typename T, AllocatorCategory Category>
inline Vector<T, Category>::Vector(size_t size, const T& value, Allocator& allocator) :
	m_allocator(allocator),
	m_data(m_allocator.allocate<T>(max(size, defaultCapacity))),
	m_size(size),
	m_capacity(max(size, defaultCapacity))
{
	std::uninitialized_fill(begin(), end(), value);
}
template <typename T, AllocatorCategory Category>
inline Vector<T, Category>::Vector(size_t size, Allocator& allocator) :
	m_allocator(allocator),
	m_data(m_allocator.allocate<T>(max(size, defaultCapacity))),
	m_size(size),
	m_capacity(max(size, defaultCapacity))
{
	std::uninitialized_default_construct(begin(), end());
}
template <typename T, AllocatorCategory Category>
inline Vector<T, Category>::Vector(const Vector& vector) :
	Vector(vector.data(), vector.size()) // No allocator
{
}
template <typename T, AllocatorCategory Category>
inline Vector<T, Category>::Vector(Vector&& vector) :
	m_allocator(mem::getAllocator(AllocatorMemoryType::Vector, Category)),
	m_data(nullptr),
	m_size(0),
	m_capacity(0)
{
	//std::swap(m_allocator, vector.m_allocator);
	std::swap(m_data, vector.m_data);
	std::swap(m_size, vector.m_size);
	std::swap(m_capacity, vector.m_capacity);
}
template <typename T, AllocatorCategory Category>
inline Vector<T, Category>& Vector<T, Category>::operator=(const Vector& vector)
{
	resize(vector.size());
	std::copy(vector.begin(), vector.end(), begin());
	return *this;
}
template <typename T, AllocatorCategory Category>
inline Vector<T, Category>& Vector<T, Category>::operator=(Vector&& vector)
{
	//std::swap(m_allocator, vector.m_allocator);
	std::swap(m_data, vector.m_data);
	std::swap(m_size, vector.m_size);
	std::swap(m_capacity, vector.m_capacity);
	return *this;
}
template <typename T, AllocatorCategory Category>
inline Vector<T, Category>::~Vector()
{
	std::destroy(begin(), end());
	m_allocator.deallocate(m_data);
}
template <typename T, AllocatorCategory Category>
inline T& Vector<T, Category>::operator[](size_t index)
{
	AKA_ASSERT(index < m_size, "Out of range");
	return m_data[index];
}
template <typename T, AllocatorCategory Category>
inline const T& Vector<T, Category>::operator[](size_t index) const
{
	AKA_ASSERT(index < m_size, "Out of range");
	return m_data[index];
}
template <typename T, AllocatorCategory Category>
inline bool Vector<T, Category>::operator==(const Vector<T, Category>& value) const
{
	return (size() == value.size()) && std::equal(begin(), end(), value.begin());
}
template <typename T, AllocatorCategory Category>
inline bool Vector<T, Category>::operator!=(const Vector<T, Category>& value) const
{
	return (size() != value.size()) || !std::equal(begin(), end(), value.begin());
}
template <typename T, AllocatorCategory Category>
inline bool Vector<T, Category>::operator<(const Vector<T, Category>& value) const
{
	return std::lexicographical_compare(begin(), end(), value.begin(), value.end());
}
template <typename T, AllocatorCategory Category>
inline bool Vector<T, Category>::operator>(const Vector<T, Category>& value) const
{
	return value < *this;
}
template <typename T, AllocatorCategory Category>
inline bool Vector<T, Category>::operator<=(const Vector<T, Category>& value) const
{
	return !(value < *this);
}
template <typename T, AllocatorCategory Category>
inline bool Vector<T, Category>::operator>=(const Vector<T, Category>& value) const
{
	return !(*this < value);
}
template <typename T, AllocatorCategory Category>
inline T& Vector<T, Category>::append(const Vector<T, Category>& vector)
{
	return append(vector.begin(), vector.end());
}
template <typename T, AllocatorCategory Category>
inline T& Vector<T, Category>::append(const T* _start, const T* _end)
{
	AKA_ASSERT(_start != nullptr && _end != nullptr, "Invalid inputs.");
	AKA_ASSERT(_end >= _start, "Invalid range");
	T* b = begin();
	T* e = end();
	size_t size = m_size;
	size_t range = (_end - _start);
	reserve(m_size + range);
	std::uninitialized_copy(_start, _end, m_data + size);
	m_size = m_size + range;
	return last();
}
template <typename T, AllocatorCategory Category>
inline T& Vector<T, Category>::append(const T& value)
{
	size_t newSize = m_size + 1;
	reserve(newSize);
	new (end()) T(value);
	m_size = newSize;
	return last();
}
template <typename T, AllocatorCategory Category>
inline T& Vector<T, Category>::append(T&& value)
{
	size_t newSize = m_size + 1;
	reserve(newSize);
	new (end()) T(std::move(value));
	m_size = newSize;
	return last();
}
template<typename T, AllocatorCategory Category>
template<typename ...Args>
inline T& Vector<T, Category>::emplace(Args ...args)
{
	size_t off = m_size;
	resize(m_size + 1, T(std::forward<Args>(args)...));
	return last();
}
template <typename T, AllocatorCategory Category>
inline void Vector<T, Category>::remove(T* _start, T* _end)
{
	AKA_ASSERT(_start >= begin() || _start < end(), "Start not in range");
	AKA_ASSERT(_end > begin() || _end <= end(), "End not in range");
	AKA_ASSERT(_end >= _start, "Invalid range");
	std::move(_end, end(), _start);
	std::destroy(_start + (end() - _end), end());
	m_size -= (_end - _start);
}
template <typename T, AllocatorCategory Category>
inline void Vector<T, Category>::remove(T* value)
{
	remove(value, value + 1);
}
template <typename T, AllocatorCategory Category>
inline size_t Vector<T, Category>::size() const
{
	return m_size;
}
template <typename T, AllocatorCategory Category>
inline size_t Vector<T, Category>::capacity() const
{
	return m_capacity;
}
template <typename T, AllocatorCategory Category>
inline void Vector<T, Category>::resize(size_t size)
{
	if (m_size == size)
		return;
	reserve(size);
	T* newEnd = m_data + size;
	if (m_size < size)
	{
		std::uninitialized_default_construct(end(), newEnd);
	}
	else
	{
		std::destroy(newEnd, end());
	}
	m_size = size;
}
template <typename T, AllocatorCategory Category>
inline void Vector<T, Category>::resize(size_t size, const T& defaultValue)
{
	if (m_size == size)
		return;
	reserve(size);
	T* newEnd = m_data + size;
	if (m_size < size)
	{
		std::uninitialized_fill(end(), newEnd, defaultValue);
	}
	else
	{
		std::destroy(newEnd, end());
	}
	m_size = size;
}
template <typename T, AllocatorCategory Category>
inline void Vector<T, Category>::reserve(size_t size)
{
	if (size <= m_capacity)
		return;
	size_t oldCapacity = m_capacity;
	size_t newCapacity = oldCapacity + oldCapacity / 2; // * 1.5 growth
	if (newCapacity < size)
		newCapacity = size; // insufficient growth.
	T* buffer = m_allocator.allocate<T>(newCapacity);
	std::uninitialized_move(begin(), end(), buffer);
	m_allocator.deallocate(m_data);
	m_capacity = newCapacity;
	m_data = buffer;
}
template <typename T, AllocatorCategory Category>
inline void Vector<T, Category>::clear()
{
	std::destroy(begin(), end());
	m_size = 0;
}
template <typename T, AllocatorCategory Category>
inline bool Vector<T, Category>::empty() const
{
	return m_size == 0;
}
template <typename T, AllocatorCategory Category>
inline T* Vector<T, Category>::data()
{
	return m_data;
}
template <typename T, AllocatorCategory Category>
inline const T* Vector<T, Category>::data() const
{
	return m_data;
}
template <typename T, AllocatorCategory Category>
inline T& Vector<T, Category>::first()
{
	return m_data[0];
}
template <typename T, AllocatorCategory Category>
inline const T& Vector<T, Category>::first() const
{
	return m_data[0];
}
template <typename T, AllocatorCategory Category>
inline T& Vector<T, Category>::last()
{
	return m_data[m_size - 1];
}
template <typename T, AllocatorCategory Category>
inline const T& Vector<T, Category>::last() const
{
	return m_data[m_size - 1];
}
template <typename T, AllocatorCategory Category>
inline T* Vector<T, Category>::begin()
{
	return m_data;
}
template <typename T, AllocatorCategory Category>
inline T* Vector<T, Category>::end()
{
	return m_data + m_size;
}
template <typename T, AllocatorCategory Category>
inline const T* Vector<T, Category>::begin() const
{
	return m_data;
}
template <typename T, AllocatorCategory Category>
inline const T* Vector<T, Category>::end() const
{
	return m_data + m_size;
}

};
