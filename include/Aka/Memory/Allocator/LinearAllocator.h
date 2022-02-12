#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Memory/Allocator.h>

namespace aka {

class LinearAllocator final : public Allocator
{
public:
	LinearAllocator(void* chunk, size_t size);
	~LinearAllocator();

	void* allocate(size_t size, size_t alignement = 0) override;
	void deallocate(void* address, size_t size) override;
	void reset() override;
	bool contiguous() const override;
};

};
