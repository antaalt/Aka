#include <Aka/Core/Config.h>

#include <Aka/Platform/Platform.h>

namespace aka {

#if defined(AKA_ENVIRONMENT64)
static constexpr size_t FNV_Prime = 1099511628211Ui64;
static constexpr size_t FNV_offsetBasis = 1099511628211Ui64;
#elif defined(AKA_ENVIRONMENT32)
static constexpr size_t FNV_Prime = 16777619U;
static constexpr size_t FNV_offsetBasis = 16777619U;
#endif

void hash(size_t& hash, const void* data, size_t size)
{
	// Fowler Noll Vo hash algo
	const uint8_t* cdata = static_cast<const uint8_t*>(data);
	for (size_t i = 0; i < size; ++i)
	{
		size_t next = static_cast<size_t>(cdata[i]);
		hash = (hash ^ next) * FNV_Prime;
	}
}

size_t hash(const void* data, size_t size)
{
	size_t s = FNV_offsetBasis;
	hash(s, data, size);
	return s;
}

};