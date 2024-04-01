#include <Aka/Core/Hash.hpp>

#include <Aka/Platform/Platform.h>

namespace aka {

namespace hash {
#if defined(AKA_ENVIRONMENT64)
static constexpr size_t FNV_Prime = 1099511628211Ui64;
static constexpr size_t FNV_offsetBasis = 1099511628211Ui64;
#elif defined(AKA_ENVIRONMENT32)
static constexpr size_t FNV_Prime = 16777619U;
static constexpr size_t FNV_offsetBasis = 16777619U;
#endif

void fnv(std::size_t& hash, const void* data, std::size_t size)
{
	// Fowler Noll Vo hash algo (1a variant)
	const uint8_t* cdata = static_cast<const uint8_t*>(data);
	for (size_t i = 0; i < size; ++i)
	{
		hash = (hash ^ static_cast<std::size_t>(cdata[i])) * FNV_Prime;
	}
}

std::size_t fnv(const void* data, std::size_t size)
{
	size_t s = FNV_offsetBasis;
	fnv(s, data, size);
	return s;
}

}; // namespace hash
}; // namespace aka