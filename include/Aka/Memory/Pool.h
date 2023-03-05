#pragma once

#include <cstdlib>
#include <stdint.h>
#include <functional>

namespace aka {

template <typename T, size_t BlockCount = 512>
class Pool final
{
public:
	Pool();
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
private:
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

		static Block* create();
	};
	size_t m_count; // Number of acquired elements
	Block* m_block; // Memory block to store objects.
	Chunk* m_freeList; // Free list for next available item in the pool.
};


template <typename T, size_t BlockCount>
inline Pool<T, BlockCount>::Pool() :
	m_count(0),
	m_block(Block::create()),
	m_freeList(nullptr)
{
	m_freeList = m_block->chunks;
}

template <typename T, size_t BlockCount>
typename Pool<T, BlockCount>::Block* Pool<T, BlockCount>::Block::create()
{
	Block* block = new Block;
	memset(block->used, 0, sizeof(block->used));
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
		delete currentBlock;
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
		lastBlock->next = Block::create();
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
};