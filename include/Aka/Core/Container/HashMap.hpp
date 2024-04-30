#pragma once

#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

#include <Aka/Core/Container/StlAllocator.hpp>

namespace aka {


template <typename K, typename T, typename Comp = ::std::less<K>> 
using TreeMap = ::std::map<K, T, Comp, AkaStlAllocator<::std::pair<const K, T>, AllocatorMemoryType::Map, AllocatorCategory::Global>>;

template <typename K, typename T, typename Hasher = ::std::hash<K>, typename Equal = ::std::equal_to<K>>
using HashMap = ::std::unordered_map<K, T, Hasher, Equal, AkaStlAllocator<::std::pair<const K, T>, AllocatorMemoryType::Map, AllocatorCategory::Global>>;


//template <typename Key, typename Type>
//using LinkedHashMap = std::map<Key, Type>;


template <typename Type, typename Hasher = ::std::hash<Type>, typename Equal = ::std::equal_to<Type>>
using HashSet = std::unordered_set<Type, Hasher, Equal, AkaStlAllocator<Type, AllocatorMemoryType::Set, AllocatorCategory::Global>>;

template <typename Type, typename Comp = ::std::less<Type>>
using TreeSet = std::set<Type, Comp, AkaStlAllocator<Type, AllocatorMemoryType::Set, AllocatorCategory::Global>>;

}