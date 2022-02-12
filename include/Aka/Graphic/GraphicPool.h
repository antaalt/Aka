#pragma once

#include <cstdlib>

namespace aka {

template <typename T>
class GraphicPool
{
public:
	GraphicPool();
	GraphicPool(size_t count);
	GraphicPool(const GraphicPool& pool) = delete;
	GraphicPool& operator=(const GraphicPool& pool) = delete;
	GraphicPool(GraphicPool&& pool);
	GraphicPool& operator=(GraphicPool&& pool);
	~GraphicPool();

	// Acquire an element
	T* acquire();
	// Release an element
	void release(T* element);
	// Reset the pool.
	void reset();
private:
	void* m_memory; // Memory stored.
	size_t m_count; // Max number of object supported.
	union Chunk { Chunk* next; T element; };
	Chunk* m_freeList; // Free list for next available item in the pool.
};

template<typename T>
inline GraphicPool<T>::GraphicPool() :
	GraphicPool(256)
{
}

template<typename T>
inline GraphicPool<T>::GraphicPool(size_t count) :
	m_memory(malloc(count * sizeof(Chunk))),
	m_count(count),
	m_freeList(nullptr)
{
	reset();
}

template<typename T>
inline GraphicPool<T>::GraphicPool(GraphicPool&& pool) :
	m_memory(nullptr),
	m_count(0),
	m_freeList(nullptr)
{
	std::swap(m_memory, pool.m_memory);
	std::swap(m_count, pool.m_count);
	std::swap(m_freeList, pool.m_freeList);
}

template<typename T>
inline GraphicPool<T>& GraphicPool<T>::operator=(GraphicPool<T>&& pool)
{
	std::swap(m_memory, pool.m_memory);
	std::swap(m_count, pool.m_count);
	std::swap(m_freeList, pool.m_freeList);
}

template<typename T>
inline GraphicPool<T>::~GraphicPool()
{
	free(m_memory);
}

template<typename T>
inline T* GraphicPool<T>::acquire()
{
	if (m_freeList == nullptr)
		return nullptr; // No more space available in pool
	Chunk* p = m_freeList;
	m_freeList = m_freeList->next;
	return &p->element;
}

template<typename T>
inline void GraphicPool<T>::release(T* element)
{
	AKA_ASSERT(element >= m_memory && (uintptr_t)element < (uintptr_t)m_memory + m_count * sizeof(Chunk), "Invalid range");
	Chunk* chunk = reinterpret_cast<Chunk*>(element);
	chunk->next = m_freeList;
	m_freeList = chunk;
}

template<typename T>
inline void GraphicPool<T>::reset()
{
	// Init free list.
	m_freeList = static_cast<Chunk*>(m_memory);
	for (size_t index = 0; index < m_count - 1; index++)
		m_freeList[index].next = &m_freeList[index + 1];
	m_freeList[m_count - 1].next = nullptr;
}

};