#pragma once

#include <memory>

#include <Aka/Core/Config.h>
#include <Aka/Memory/Memory.h>

namespace aka {

template <typename T>
class List final
{
	using AllocatorType = Allocator;
	static const size_t defaultCapacity = 16;
	struct Node {
		T value;
		Node* prev;
		Node* next;
	};
public:
	// Cant use raw data as iterator, memory not continuous
	struct Iterator {

	};
	struct ConstIterator {

	};
public:
	List();
	explicit List(AllocatorType& allocator);
	List(const List& list, AllocatorType& allocator = mem::getAllocator(AllocatorMemoryType::List, AllocatorCategory::Global));
	List(List&& list, AllocatorType& allocator = mem::getAllocator(AllocatorMemoryType::List, AllocatorCategory::Global));
	List& operator=(const List& list);
	List& operator=(List&& list);
	~List();

	T& operator[](size_t index);
	const T& operator[](size_t index) const;

	bool operator==(const List<T>& list) const;
	bool operator!=(const List<T>& list) const;
	bool operator<(const List<T>& list) const;
	bool operator>(const List<T>& list) const;
	bool operator<=(const List<T>& list) const;
	bool operator>=(const List<T>& list) const;

	T& append(const List<T>& list);
	T& append(const T& value);
	T& append(T&& value);
	template <typename ...Args>
	T& emplace(Args ...args);
	T& prepend(const List<T>& list);
	T& prepend(const T& value);
	T& prepend(T&& value);
	
	void insert(T* position, const T& value);

	void remove(T* value);

	// Get the size of the list
	size_t size() const;
	// Resize the list
	void resize(size_t size);
	// Resize the list capacity
	void reserve(size_t size);
	// Empty the list
	void clear();
	// Check if list empty
	bool empty() const;

	// Pointer to the list
	Iterator data();
	// Pointer to the list
	ConstIterator data() const;
	// First element of list
	Iterator first();
	// First element of list
	ConstIterator first() const;
	// Last element of list
	Iterator last();
	// Last element of list
	ConstIterator last() const;

	// Pointer to beginning of list
	Iterator begin();
	// Pointer to ending of list
	Iterator end();
	// Pointer to beginning of list
	ConstIterator begin() const;
	// Pointer to ending of list
	ConstIterator end() const;
private:
	AllocatorType& m_allocator;
	T* m_first;
	T* m_last;
	// TODO: use a pool & free list of Node to add them.
	// Issue: when resizing pool, this will invalide all prev & next pointers...
};

template <typename T>
inline List<T>::List() :
	List(mem::getAllocator(AllocatorMemoryType::Persistent, AllocatorCategory::List))
{
}
template<typename T>
inline List<T>::List(AllocatorType& allocator) :
	m_allocator(mem::getAllocator(AllocatorMemoryType::Persistent, AllocatorCategory::List)),
	m_first(nullptr),
	m_last(nullptr)
{
}
template <typename T>
inline List<T>::List(const List& list, AllocatorType& allocator) :
	List(list.data(), list.size(), allocator)
{
}
template <typename T>
inline List<T>::List(List&& list, AllocatorType& allocator) :
	m_allocator(allocator),
	m_data(nullptr),
	m_size(0),
	m_capacity(0)
{
	//std::swap(m_allocator, list.m_allocator);
	std::swap(m_data, list.m_data);
	std::swap(m_size, list.m_size);
	std::swap(m_capacity, list.m_capacity);
}
template <typename T>
inline List<T>& List<T>::operator=(const List& list)
{
	resize(list.size());
	std::copy(list.data(), list.data() + list.size(), begin());
	return *this;
}
template <typename T>
inline List<T>& List<T>::operator=(List&& list)
{
	//std::swap(m_allocator, list.m_allocator);
	std::swap(m_data, list.m_data);
	std::swap(m_size, list.m_size);
	std::swap(m_capacity, list.m_capacity);
	return *this;
}
template <typename T>
inline List<T>::~List()
{
	if (m_capacity > 0)
	{
		std::destroy(begin(), end());
		m_allocator.deallocate(m_data, m_capacity);
	}
}
template <typename T>
inline T& List<T>::operator[](size_t index)
{
	AKA_ASSERT(index < m_size, "Out of range");
	return m_data[index];
}
template <typename T>
inline const T& List<T>::operator[](size_t index) const
{
	AKA_ASSERT(index < m_size, "Out of range");
	return m_data[index];
}
template <typename T>
inline bool List<T>::operator==(const List<T>& value) const
{
	return (size() == value.size()) && std::equal(begin(), end(), value.begin());
}
template <typename T>
inline bool List<T>::operator!=(const List<T>& value) const
{
	return (size() != value.size()) || !std::equal(begin(), end(), value.begin());
}
template <typename T>
inline bool List<T>::operator<(const List<T>& value) const
{
	return std::lexicographical_compare(begin(), end(), value.begin(), value.end());
}
template <typename T>
inline bool List<T>::operator>(const List<T>& value) const
{
	return value < *this;
}
template <typename T>
inline bool List<T>::operator<=(const List<T>& value) const
{
	return !(value < *this);
}
template <typename T>
inline bool List<T>::operator>=(const List<T>& value) const
{
	return !(*this < value);
}
template <typename T>
inline T& List<T>::append(const List<T>& list)
{
	return append(list.data(), list.data() + list.size());
}
template <typename T>
inline T& List<T>::append(const T& value)
{
	size_t off = m_size;
	resize(m_size + 1; value);
	return last();
}
template <typename T>
inline T& List<T>::append(T&& value)
{
	size_t off = m_size;
	resize(m_size + 1; T(std::move(value));
	return last();
}
template<typename T>
template<typename ...Args>
inline T& List<T>::emplace(Args ...args)
{
	size_t off = m_size;
	resize(m_size + 1; T(std::forward<Args>(args)...));
	return last();
}
template <typename T>
inline void List<T>::remove(T* start, T* end)
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
inline void List<T>::remove(T* value)
{
	remove(value, value + 1);
}
template <typename T>
inline size_t List<T>::size() const
{
	return m_size;
}
template <typename T>
inline size_t List<T>::capacity() const
{
	return m_capacity;
}
template <typename T>
inline void List<T>::resize(size_t size)
{
	if (m_size == size)
		return;
	reserve(size);
	m_size = size;
}
template <typename T>
inline void List<T>::reserve(size_t size)
{
	if (size <= m_capacity)
		return;
	T* b = begin();
	T* e = end();
	size_t oldCapacity = m_capacity;
	m_capacity = size;
	T* buffer = m_allocator.allocate<T>(m_capacity);
	std::uninitialized_move(b, e, buffer);
	std::destroy(b, e); // needed ?
	m_allocator.deallocate(m_data, oldCapacity);
	m_data = buffer;
}
template <typename T>
inline void List<T>::clear()
{
	m_size = 0;
}
template <typename T>
inline bool List<T>::empty() const
{
	return m_size == 0;
}
template <typename T>
inline T* List<T>::data()
{
	return m_data;
}
template <typename T>
inline const T* List<T>::data() const
{
	return m_data;
}
template <typename T>
inline T& List<T>::first()
{
	return m_data[0];
}
template <typename T>
inline const T& List<T>::first() const
{
	return m_data[0];
}
template <typename T>
inline T& List<T>::last()
{
	return m_data[m_size - 1];
}
template <typename T>
inline const T& List<T>::last() const
{
	return m_data[m_size - 1];
}
template <typename T>
inline T* List<T>::begin()
{
	return m_data;
}
template <typename T>
inline T* List<T>::end()
{
	return m_data + m_size;
}
template <typename T>
inline const T* List<T>::begin() const
{
	return m_data;
}
template <typename T>
inline const T* List<T>::end() const
{
	return m_data + m_size;
}

};
