#pragma once

#include <cstdlib>
#include <stdint.h>
#include <functional>

#include <Aka/Memory/Memory.h>

namespace aka {

template <typename T, size_t BlockCount>
class Pool;

template <typename T, size_t BlockCount = 256>
class PoolIterator {
public:
	explicit PoolIterator(typename Pool<T, BlockCount>::Block* _block, typename Pool<T, BlockCount>::Chunk* _begin);
	PoolIterator<T, BlockCount>& operator++();
	PoolIterator<T, BlockCount> operator++(int);
	T& operator*();
	const T& operator*() const;
	bool operator==(const PoolIterator<T, BlockCount>& value) const;
	bool operator!=(const PoolIterator<T, BlockCount>& value) const;

private:
	size_t isChunkUsed(typename Pool<T, BlockCount>::Chunk* _chunk) const;
	size_t getBlockIndex() const;
	size_t isBlockFinished() const;
	bool isLastBlock() const;
private:
	typename Pool<T, BlockCount>::Chunk* m_current;
	typename Pool<T, BlockCount>::Block* m_block;
};


template <typename T>
class PoolRange 
{
public:
	PoolRange(PoolIterator<T> _begin, PoolIterator<T> _end) : m_begin(_begin), m_end(_end) {}
	PoolIterator<T> begin() { return m_begin; }
	PoolIterator<T> end() { return m_end; }
private:
	PoolIterator<T> m_begin, m_end;
};

template <typename T, size_t BlockCount = 256>
class Pool final
{
	using AllocatorType = Allocator;
public:
	Pool();
	Pool(AllocatorType& allocator);
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
	// Release all elements with custom deleter
	void release(std::function<void(T&)>&& deleter);
	// Return number of acquired elements
	size_t count() const;
public:
	// Return begin iterator
	PoolIterator<T, BlockCount> begin();
	// Return end iterator
	PoolIterator<T, BlockCount> end();
private:
	friend class PoolIterator<T, BlockCount>;
	union Chunk {
		Chunk() {}
		~Chunk() {} 
		Chunk* next; 
		T element; 
	}; // TODO align this
	struct Block {
		Block() {}
		~Block() {}
		Chunk chunks[BlockCount];
		uint8_t used[(BlockCount + 7) / 8]; // Bitmask for every element to check if it is currently used;
		Block* next; // Next block if not enough space

		static Block* create(AllocatorType& _allocator);
	};
	AllocatorType& m_allocator;
	size_t m_count; // Number of acquired elements
	Block* m_block; // Memory block to store objects.
	Chunk* m_freeList; // Free list for next available item in the pool.
};


template <typename T, size_t BlockCount>
inline Pool<T, BlockCount>::Pool() :
	Pool(mem::getAllocator(AllocatorMemoryType::Persistent, AllocatorCategory::Pool))
{
}


template <typename T, size_t BlockCount>
inline Pool<T, BlockCount>::Pool(AllocatorType& _allocator) :
	m_allocator(_allocator),
	m_count(0),
	m_block(Block::create(m_allocator)),
	m_freeList(nullptr)
{
	m_freeList = m_block->chunks;
}

template <typename T, size_t BlockCount>
typename Pool<T, BlockCount>::Block* Pool<T, BlockCount>::Block::create(AllocatorType& _allocator)
{
	
	Block* block = static_cast<Block*>(_allocator.allocate<Block>(1));
	Memory::set(block->used, 0, sizeof(block->used));
	block->next = nullptr;
	// Init data free list.
	Chunk* start = block->chunks;
	for (size_t index = 0; index < BlockCount; index++)
		block->chunks[index].next = ++start;
	block->chunks[BlockCount - 1].next = nullptr;
	return block;
}

template <typename T, size_t BlockCount>
inline Pool<T, BlockCount>::~Pool()
{
	// Release all objects
	release();
	// Free memory
	Block* nextBlock = m_block;
	do
	{
		Block* currentBlock = nextBlock;
		nextBlock = currentBlock->next;
		m_allocator.deallocate(currentBlock, 1);
	} while (nextBlock != nullptr);
}
template <typename T, size_t BlockCount>
template<typename ...Args>
inline T* Pool<T, BlockCount>::acquire(Args&&... args)
{
	Block* block = m_block;
	if (m_freeList == nullptr)
	{
		// Create new block and update freelist.
		Block* lastBlock = m_block;
		while (lastBlock->next != nullptr)
			lastBlock = lastBlock->next;
		lastBlock->next = Block::create(m_allocator);
		m_freeList = lastBlock->next->chunks;
		block = lastBlock->next;
	}
	else
	{
		// Find block used by freelist currently
		while (block != nullptr && (((uintptr_t)m_freeList < (uintptr_t)block->chunks) || ((uintptr_t)m_freeList > ((uintptr_t)&block->chunks[BlockCount - 1]))))
			block = block->next;
		AKA_ASSERT(block != nullptr, "m_freeList not in any block");
	}
	Chunk* p = m_freeList;
	m_freeList = m_freeList->next;
	new (&p->element) T(std::forward<Args>(args)...); // Call constructor with new placement
	// Set used bitmask
	size_t index = p - block->chunks;
	block->used[index / 8] |= 0x01 << (index % 8);
	// Register the new element
	m_count++;
	return &p->element;
}

template <typename T, size_t BlockCount>
inline void Pool<T, BlockCount>::release(T* element)
{
	Block* block = m_block;
	while (block != nullptr && (((uintptr_t)element < (uintptr_t)block->chunks) || ((uintptr_t)element > ((uintptr_t)&block->chunks[BlockCount - 1]))))
		block = block->next;
	AKA_ASSERT(block != nullptr, "m_freeList not in any block");
	//if (block == nullptr)
	//	return; // Element not from pool

	Chunk* chunk = reinterpret_cast<Chunk*>(element);
	size_t index = chunk - block->chunks;
	if (block->used[index / 8] & (0x01 << (index % 8)))
	{
		if constexpr (std::is_destructible<T>::value)
			element->~T(); // Call destructor
		chunk->next = m_freeList;
		m_freeList = chunk;
		// Set used bitmask
		block->used[index / 8] &= ~(0x01 << (index % 8));
		// Unregister the new element
		AKA_ASSERT(m_count > 0, "System error");
		m_count--;
	}
}

template <typename T, size_t BlockCount>
inline void Pool<T, BlockCount>::release()
{
	size_t count = 0;
	Block* currentBlock = m_block;
	do
	{
		if constexpr (std::is_destructible<T>::value)
		{
			Chunk* chunks = currentBlock->chunks;
			// Call destructors
			for (size_t index = 0; index < BlockCount; index++)
			{
				if (currentBlock->used[index / 8] & (0x01 << (index % 8)))
				{
					chunks[index].element.~T();
					count++;
				}
			}
			// Reset freelist content
			m_freeList = chunks;
			for (size_t index = 0; index < BlockCount; index++)
				m_freeList[index].next = ++chunks;
			m_freeList[BlockCount - 1].next = currentBlock->next == nullptr ? nullptr : currentBlock->next->chunks;
		}
		// Set used bitmask
		memset(currentBlock->used, 0, sizeof(currentBlock->used));

		currentBlock = currentBlock->next;
	} while (currentBlock != nullptr);

	// Reset freelist start
	m_freeList = m_block->chunks;
	AKA_ASSERT(m_count == count, "Invalid count");
	m_count = 0;
}
template <typename T, size_t BlockCount>
inline void Pool<T, BlockCount>::release(std::function<void(T&)>&& deleter)
{
	size_t count = 0;
	Block* currentBlock = m_block;
	do
	{
		Chunk* chunks = currentBlock->chunks;
		// Call destructors
		for (size_t index = 0; index < BlockCount; index++)
		{
			if (currentBlock->used[index / 8] & (0x01 << (index % 8)))
			{
				// Release before calling deleter because it might call release too.
				currentBlock->used[index / 8] &= ~(0x01 << (index % 8));
				deleter(chunks[index].element);
				count++;
			}
		}
		// Reset freelist content
		m_freeList = chunks;
		for (size_t index = 0; index < BlockCount; index++)
			m_freeList[index].next = ++chunks;
		m_freeList[BlockCount - 1].next = currentBlock->next == nullptr ? nullptr : currentBlock->next->chunks;

		// Set used bitmask
		memset(currentBlock->used, 0, sizeof(currentBlock->used));

		currentBlock = currentBlock->next;
	} while (currentBlock != nullptr);

	// Reset freelist start
	m_freeList = m_block->chunks;
	AKA_ASSERT(m_count == count, "Invalid count");
	m_count = 0;
}

template<typename T, size_t BlockCount>
inline size_t aka::Pool<T, BlockCount>::count() const
{
	return m_count;
}
template<typename T, size_t BlockCount>
PoolIterator<T, BlockCount> Pool<T, BlockCount>::begin() {
	return PoolIterator<T, BlockCount>(m_block, m_block[0].chunks);
}
template<typename T, size_t BlockCount>
PoolIterator<T, BlockCount> Pool<T, BlockCount>::end() {
	Block* endBlock = m_block;
	while (endBlock->next != nullptr)
		endBlock = endBlock->next;
	return PoolIterator<T, BlockCount>(endBlock, endBlock->chunks + BlockCount);
}

template<typename T, size_t BlockCount>
PoolIterator<T, BlockCount>::PoolIterator(typename Pool<T, BlockCount>::Block* _block, typename Pool<T, BlockCount>::Chunk* _begin) : 
	m_current(_begin), 
	m_block(_block) 
{
}
template<typename T, size_t BlockCount>
PoolIterator<T, BlockCount>& PoolIterator<T, BlockCount>::operator++()
{
	do {
		m_current++;
		if (isBlockFinished())
		{
			if (!isLastBlock())
			{
				m_block = m_block->next;
				m_current = m_block->chunks;
			}
			else
			{
				return *this;
			}
		}
	} while (!isBlockFinished() && !isChunkUsed(m_current));
	return *this;
}
template<typename T, size_t BlockCount>
PoolIterator<T, BlockCount> PoolIterator<T, BlockCount>::operator++(int)
{
	PoolIterator old = *this;
	++(*this);
	return old;
}
template<typename T, size_t BlockCount>
T& PoolIterator<T, BlockCount>::operator*() 
{
	return m_current->element; 
}
template<typename T, size_t BlockCount>
const T& PoolIterator<T, BlockCount>::operator*() const 
{ 
	return m_current->element; 
}
template<typename T, size_t BlockCount>
bool PoolIterator<T, BlockCount>::operator==(const PoolIterator<T, BlockCount>& value) const 
{
	return value.m_current == m_current; 
}
template<typename T, size_t BlockCount>
bool PoolIterator<T, BlockCount>::operator!=(const PoolIterator<T, BlockCount>& value) const
{ 
	return value.m_current != m_current;
}
template<typename T, size_t BlockCount>
size_t PoolIterator<T, BlockCount>::isChunkUsed(typename Pool<T, BlockCount>::Chunk* _chunk) const 
{
	return ((m_block->used[getBlockIndex() / 8] >> (getBlockIndex() & 7)) & 0x1) == 0x1; 
}
template<typename T, size_t BlockCount>
size_t PoolIterator<T, BlockCount>::getBlockIndex() const 
{ 
	return m_current - m_block->chunks; 
}
template<typename T, size_t BlockCount>
size_t PoolIterator<T, BlockCount>::isBlockFinished() const 
{ 
	return getBlockIndex() >= BlockCount; 
}
template<typename T, size_t BlockCount>
bool PoolIterator<T, BlockCount>::isLastBlock() const 
{ 
	return m_block->next == nullptr; 
}

};