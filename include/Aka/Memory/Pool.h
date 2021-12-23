#pragma once

#include <cstdlib>
#include <stdint.h>

namespace aka {

template <typename T>
class Pool final
{
public:
	Pool();
	Pool(size_t count);
	Pool(const Pool&) = delete;
	Pool& operator=(const Pool&) = delete;
	Pool(Pool&&) = delete;
	Pool& operator=(Pool&&) = delete;
	~Pool();
public:
	// Acquire an element from the pool.
	template<typename ...Args> T* acquire(Args&&... args);
	// Release an element acquired from this pool.
	void release(T* element);
	// Release all elements
	void release();
	// Resize the pool
	void resize(size_t count);
private:
	void* m_mem; // Memory to store objects.
	size_t m_count; // Max number of object supported.
	union Chunk { ~Chunk() {} Chunk* next; T element; };
	Chunk* m_freeList; // Free list for next available item in the pool.
	uint8_t* m_used; // Bitmask for every element to check if it is currently used;
};


template <typename T>
inline Pool<T>::Pool() :
	Pool(16) // default capacity
{
}

template <typename T>
inline Pool<T>::Pool(size_t count) :
	m_mem(malloc(count * sizeof(Chunk))),
	m_count(count),
	m_freeList(nullptr),
	m_used(static_cast<uint8_t*>(calloc((count + 7) / 8, 1)))
{
	// Init free list.
	m_freeList = static_cast<Chunk*>(m_mem);
	Chunk* start = m_freeList;
	for (size_t index = 0; index < m_count; index++)
		m_freeList[index].next = ++start;
	m_freeList[m_count - 1].next = nullptr;
}


template <typename T>
inline Pool<T>::~Pool()
{
	release();
	free(m_mem);
	free(m_used);
}
template <typename T>
template<typename ...Args>
inline T* Pool<T>::acquire(Args&&... args)
{
	if (m_freeList == nullptr)
		return nullptr; // No more space available in pool
	Chunk* p = m_freeList;
	m_freeList = m_freeList->next;
	if constexpr (std::is_constructible<T>::value)
		new (&p->element) T(std::forward<Args>(args)...); // Call constructor with new placement
	// Set used bitmask
	size_t index = p - static_cast<Chunk*>(m_mem);
	m_used[index / 8] |= 0x01 << (index % 8);
	return &p->element;
}

template <typename T>
inline void Pool<T>::release(T* element)
{
	AKA_ASSERT(element > m_mem && (uintptr_t)element < (uintptr_t)m_mem + m_count * sizeof(Chunk), "Invalid range");
	if constexpr (std::is_destructible<T>::value)
		element->~T(); // Call destructor
	Chunk* chunk = reinterpret_cast<Chunk*>(element);
	chunk->next = m_freeList;
	m_freeList = chunk;
	// Set used bitmask
	size_t index = chunk - static_cast<Chunk*>(m_mem);
	m_used[index / 8] &= ~(0x01 << (index % 8));
}

template <typename T>
inline void Pool<T>::release()
{
	if constexpr (std::is_destructible<T>::value)
	{
		Chunk* chunks = static_cast<Chunk*>(m_mem);
		// Call destructors
		for (size_t index = 0; index < m_count; index++)
			if (m_used[index / 8] & (0x01 << (index % 8)))
				chunks[index].element.~T();
		// Reset freelist
		m_freeList = chunks;
		for (size_t index = 0; index < m_count; index++)
			m_freeList[index].next = ++chunks;
		m_freeList[m_count - 1].next = nullptr;
	}
	// Set used bitmask
	memset(m_used, 0, (m_count + 7) / 8);
}

template <typename T>
inline void Pool<T>::resize(size_t count)
{
	if (count <= m_count)
	{
		return;
	}
	else
	{
		void* mem = malloc(count * sizeof(Chunk));
		uint8_t* used = static_cast<uint8_t*>(calloc((count + 7) / 8, 1));
		memcpy(mem, m_mem, m_count * sizeof(Chunk));
		memcpy(used, m_used, (m_count + 7) / 8);
		Chunk* start = nullptr;
		if (m_freeList != nullptr)
		{
			// Reset all link to new memory
			size_t indexChunk = m_freeList - static_cast<Chunk*>(m_mem);
			Chunk* oldChunk = m_freeList;
			Chunk* newChunk = static_cast<Chunk*>(mem) + indexChunk;
			m_freeList = newChunk;
			while (oldChunk->next != nullptr)
			{
				size_t i = oldChunk->next - static_cast<Chunk*>(m_mem);
				newChunk->next = static_cast<Chunk*>(mem) + i;
				oldChunk = oldChunk->next;
				newChunk = newChunk->next;
			}
			newChunk->next = static_cast<Chunk*>(mem) + m_count;
			start = newChunk;
		}
		else
		{
			m_freeList = static_cast<Chunk*>(mem) + m_count;
			m_freeList->next = nullptr;
			start = m_freeList;
		}
		free(m_mem);
		free(m_used);
		m_mem = mem;
		m_used = used;

		// Set next value for new created chunks
		size_t diff = count - m_count;
		for (size_t index = m_count; index < count - 1; index++, start++)
			start->next = start + 1;
		start->next = nullptr;
		m_count = count;
	}
}

};