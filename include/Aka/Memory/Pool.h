#pragma once

#include <cstdlib>
#include <stdint.h>
#include <functional>

#include <Aka/Memory/Memory.h>

namespace aka {

template <typename T, size_t ChunkCountPerBlock>
class Pool;

template <typename T, size_t ChunkCountPerBlock = 256>
class PoolIterator {
public:
	explicit PoolIterator(typename Pool<T, ChunkCountPerBlock>::Block* _block, typename Pool<T, ChunkCountPerBlock>::Chunk* _begin);
	PoolIterator<T, ChunkCountPerBlock>& operator++();
	PoolIterator<T, ChunkCountPerBlock> operator++(int);
	T& operator*();
	const T& operator*() const;
	bool operator==(const PoolIterator<T, ChunkCountPerBlock>& value) const;
	bool operator!=(const PoolIterator<T, ChunkCountPerBlock>& value) const;

private:
	size_t isBlockFinished() const;
	bool isLastBlock() const;
private:
	typename Pool<T, ChunkCountPerBlock>::Chunk* m_current;
	typename Pool<T, ChunkCountPerBlock>::Block* m_block;
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

template <typename T, size_t ChunkCountPerBlock = 256>
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
	PoolIterator<T, ChunkCountPerBlock> begin();
	// Return end iterator
	PoolIterator<T, ChunkCountPerBlock> end();
private:
	friend class PoolIterator<T, ChunkCountPerBlock>;
	union Chunk {
		Chunk() {}
		~Chunk() {} 
		Chunk* m_next;
		T m_element; 
	}; // TODO align this
	struct Block {
		Block() {}
		~Block() {}
		Chunk m_chunks[ChunkCountPerBlock];
		uint8_t m_used[(ChunkCountPerBlock + 7) / 8]; // Bitmask for every element to check if it is currently used;
		Block* m_next; // Next block if not enough space
		size_t m_count; // Number of acquired elements for block

		static Block* create(AllocatorType& _allocator);
		// Check if chunk is used within block
		bool isChunkUsed(const Chunk* _chunk) const;
		// Get index of chunk in block
		size_t getChunkIndex(const Chunk* _chunk) const;
	};
	AllocatorType& m_allocator;
	Block* m_block; // Memory block to store objects.
	Chunk* m_freeList; // Free list for next available item in the pool.
};


template <typename T, size_t ChunkCountPerBlock>
inline Pool<T, ChunkCountPerBlock>::Pool() :
	Pool(mem::getAllocator(AllocatorMemoryType::Persistent, AllocatorCategory::Pool))
{
}


template <typename T, size_t ChunkCountPerBlock>
inline Pool<T, ChunkCountPerBlock>::Pool(AllocatorType& _allocator) :
	m_allocator(_allocator),
	m_block(Block::create(m_allocator)),
	m_freeList(nullptr)
{
	m_freeList = m_block->m_chunks;
}

template <typename T, size_t ChunkCountPerBlock>
typename Pool<T, ChunkCountPerBlock>::Block* Pool<T, ChunkCountPerBlock>::Block::create(AllocatorType& _allocator)
{
	
	Block* block = static_cast<Block*>(_allocator.allocate<Block>(1));
	Memory::set(block->m_used, 0, sizeof(block->m_used));
	block->m_next = nullptr;
	block->m_count = 0;
	// Init data free list.
	Chunk* start = block->m_chunks;
	for (size_t index = 0; index < ChunkCountPerBlock; index++)
		block->m_chunks[index].m_next = ++start;
	block->m_chunks[ChunkCountPerBlock - 1].m_next = nullptr;
	return block;
}

template<typename T, size_t ChunkCountPerBlock>
inline bool Pool<T, ChunkCountPerBlock>::Block::isChunkUsed(const Chunk* _chunk) const
{
	size_t chunkIndex = getChunkIndex(_chunk);
	return ((m_used[chunkIndex / 8] >> (chunkIndex & 7)) & 0x1) == 0x1;
}

template<typename T, size_t ChunkCountPerBlock>
inline size_t Pool<T, ChunkCountPerBlock>::Block::getChunkIndex(const Chunk* _chunk) const
{
	intptr_t chunkIndex = _chunk - m_chunks;
	AKA_ASSERT(chunkIndex >= 0, "Invalid input");
	return (size_t)chunkIndex;
}

template <typename T, size_t ChunkCountPerBlock>
inline Pool<T, ChunkCountPerBlock>::~Pool()
{
	// Release all objects
	release();
	// Free memory
	Block* nextBlock = m_block;
	do
	{
		Block* currentBlock = nextBlock;
		nextBlock = currentBlock->m_next;
		m_allocator.deallocate(currentBlock, 1);
	} while (nextBlock != nullptr);
}
template <typename T, size_t ChunkCountPerBlock>
template<typename ...Args>
inline T* Pool<T, ChunkCountPerBlock>::acquire(Args&&... args)
{
	Block* block = m_block;
	if (m_freeList == nullptr)
	{
		// Create new block and update freelist.
		Block* lastBlock = m_block;
		while (lastBlock->m_next != nullptr)
			lastBlock = lastBlock->m_next;
		lastBlock->m_next = Block::create(m_allocator);
		m_freeList = lastBlock->m_next->m_chunks;
		block = lastBlock->m_next;
	}
	else
	{
		// Find block used by freelist currently
		while (block != nullptr && (((uintptr_t)m_freeList < (uintptr_t)block->m_chunks) || ((uintptr_t)m_freeList > ((uintptr_t)&block->m_chunks[ChunkCountPerBlock - 1]))))
			block = block->m_next;
		AKA_ASSERT(block != nullptr, "m_freeList not in any block");
	}
	Chunk* p = m_freeList;
	m_freeList = m_freeList->m_next;
	new (&p->m_element) T(std::forward<Args>(args)...); // Call constructor with new placement
	// Set used bitmask
	size_t index = p - block->m_chunks;
	block->m_used[index / 8] |= 0x01 << (index % 8);
	// Register the new element
	block->m_count++;
	return &p->m_element;
}

template <typename T, size_t ChunkCountPerBlock>
inline void Pool<T, ChunkCountPerBlock>::release(T* element)
{
	Block* block = m_block;
	while (block != nullptr && (((uintptr_t)element < (uintptr_t)block->m_chunks) || ((uintptr_t)element > ((uintptr_t)&block->m_chunks[ChunkCountPerBlock - 1]))))
		block = block->m_next;
	AKA_ASSERT(block != nullptr, "m_freeList not in any block");
	//if (block == nullptr)
	//	return; // Element not from pool

	Chunk* chunk = reinterpret_cast<Chunk*>(element);
	size_t index = chunk - block->m_chunks;
	if (block->m_used[index / 8] & (0x01 << (index % 8)))
	{
		if constexpr (std::is_destructible<T>::value)
			element->~T(); // Call destructor
		chunk->m_next = m_freeList;
		m_freeList = chunk;
		// Set used bitmask
		block->m_used[index / 8] &= ~(0x01 << (index % 8));
		// Unregister the new element
		AKA_ASSERT(block->m_count > 0, "System error");
		block->m_count--;
	}
}

template <typename T, size_t ChunkCountPerBlock>
inline void Pool<T, ChunkCountPerBlock>::release()
{
	Block* currentBlock = m_block;
	do
	{
		size_t count = 0;
		if constexpr (std::is_destructible<T>::value)
		{
			Chunk* chunks = currentBlock->m_chunks;
			// Call destructors
			for (size_t index = 0; index < ChunkCountPerBlock; index++)
			{
				if (currentBlock->m_used[index / 8] & (0x01 << (index % 8)))
				{
					chunks[index].m_element.~T();
					count++;
				}
			}
			// Reset freelist content
			m_freeList = chunks;
			for (size_t index = 0; index < ChunkCountPerBlock; index++)
				m_freeList[index].m_next = ++chunks;
			m_freeList[ChunkCountPerBlock - 1].m_next = currentBlock->m_next == nullptr ? nullptr : currentBlock->m_next->m_chunks;
		}
		// Set used bitmask
		memset(currentBlock->m_used, 0, sizeof(currentBlock->m_used));

		AKA_ASSERT(currentBlock->m_count == count, "Invalid count");
		currentBlock->m_count = 0;

		currentBlock = currentBlock->m_next;
	} while (currentBlock != nullptr);

	// Reset freelist start
	m_freeList = m_block->m_chunks;
}
template <typename T, size_t ChunkCountPerBlock>
inline void Pool<T, ChunkCountPerBlock>::release(std::function<void(T&)>&& deleter)
{
	size_t count = 0;
	Block* currentBlock = m_block;
	do
	{
		Chunk* chunks = currentBlock->m_chunks;
		// Call destructors
		for (size_t index = 0; index < ChunkCountPerBlock; index++)
		{
			if (currentBlock->m_used[index / 8] & (0x01 << (index % 8)))
			{
				// Release before calling deleter because it might call release too.
				currentBlock->m_used[index / 8] &= ~(0x01 << (index % 8));
				deleter(chunks[index].m_element);
				count++;
			}
		}
		// Reset freelist content
		m_freeList = chunks;
		for (size_t index = 0; index < ChunkCountPerBlock; index++)
			m_freeList[index].m_next = ++chunks;
		m_freeList[ChunkCountPerBlock - 1].m_next = currentBlock->m_next == nullptr ? nullptr : currentBlock->m_next->m_chunks;

		// Set used bitmask
		memset(currentBlock->m_used, 0, sizeof(currentBlock->m_used));

		AKA_ASSERT(currentBlock->m_count == count, "Invalid count");
		currentBlock->m_count = 0;

		currentBlock = currentBlock->m_next;
	} while (currentBlock != nullptr);

	// Reset freelist start
	m_freeList = m_block->m_chunks;
}

template<typename T, size_t ChunkCountPerBlock>
inline size_t aka::Pool<T, ChunkCountPerBlock>::count() const
{
	Block* currentBlock = m_block;
	size_t count = 0;
	do
	{
		count += currentBlock->m_count;
		currentBlock = currentBlock->m_next;
	} while (currentBlock != nullptr);
	return count;
}
template<typename T, size_t ChunkCountPerBlock>
PoolIterator<T, ChunkCountPerBlock> Pool<T, ChunkCountPerBlock>::begin() {
	return PoolIterator<T, ChunkCountPerBlock>(m_block, m_block->m_chunks);
}
template<typename T, size_t ChunkCountPerBlock>
PoolIterator<T, ChunkCountPerBlock> Pool<T, ChunkCountPerBlock>::end() {
	Block* endBlock = m_block;
	while (endBlock->m_next != nullptr)
		endBlock = endBlock->m_next;

	// TODO: need to take the last NON EMPTY block or it will be an issue.
	// Could move count to block instead of pool for that.
	Chunk* endChunk = endBlock->m_chunks;
	/*Chunk* currentChunk = endChunk;
	do
	{
		if (endBlock->isChunkUsed(currentChunk))
			endChunk = currentChunk + 1; // End is after last one.
	}
	while (++currentChunk - endBlock->m_chunks < ChunkCountPerBlock);*/

	if (endBlock->m_count == 0)
		return PoolIterator<T, ChunkCountPerBlock>(endBlock, endChunk);
	else
		return PoolIterator<T, ChunkCountPerBlock>(endBlock, endChunk + ChunkCountPerBlock);
}

template<typename T, size_t ChunkCountPerBlock>
PoolIterator<T, ChunkCountPerBlock>::PoolIterator(typename Pool<T, ChunkCountPerBlock>::Block* _block, typename Pool<T, ChunkCountPerBlock>::Chunk* _begin) : 
	m_current(_begin), 
	m_block(_block) 
{
}
template<typename T, size_t ChunkCountPerBlock>
PoolIterator<T, ChunkCountPerBlock>& PoolIterator<T, ChunkCountPerBlock>::operator++()
{
	do {
		m_current++;
		if (isBlockFinished())
		{
			if (!isLastBlock())
			{
				m_block = m_block->m_next;
				m_current = m_block->m_chunks;
			}
			else
			{
				return *this;
			}
		}
	} while (!isBlockFinished() && !m_block->isChunkUsed(m_current));
	return *this;
}
template<typename T, size_t ChunkCountPerBlock>
PoolIterator<T, ChunkCountPerBlock> PoolIterator<T, ChunkCountPerBlock>::operator++(int)
{
	PoolIterator old = *this;
	++(*this);
	return old;
}
template<typename T, size_t ChunkCountPerBlock>
T& PoolIterator<T, ChunkCountPerBlock>::operator*() 
{
	return m_current->m_element; 
}
template<typename T, size_t ChunkCountPerBlock>
const T& PoolIterator<T, ChunkCountPerBlock>::operator*() const 
{ 
	return m_current->m_element; 
}
template<typename T, size_t ChunkCountPerBlock>
bool PoolIterator<T, ChunkCountPerBlock>::operator==(const PoolIterator<T, ChunkCountPerBlock>& value) const 
{
	return value.m_current == m_current; 
}
template<typename T, size_t ChunkCountPerBlock>
bool PoolIterator<T, ChunkCountPerBlock>::operator!=(const PoolIterator<T, ChunkCountPerBlock>& value) const
{ 
	return value.m_current != m_current;
}
template<typename T, size_t ChunkCountPerBlock>
size_t PoolIterator<T, ChunkCountPerBlock>::isBlockFinished() const
{ 
	return m_block->getChunkIndex(m_current) >= ChunkCountPerBlock;
}
template<typename T, size_t ChunkCountPerBlock>
bool PoolIterator<T, ChunkCountPerBlock>::isLastBlock() const
{ 
	return m_block->m_next == nullptr; 
}

};