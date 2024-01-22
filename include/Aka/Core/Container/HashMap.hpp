#pragma once

#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

namespace aka {

template <typename Key, typename Type>
using HashMap = std::unordered_map<Key, Type>;

template <typename Key, typename Type>
using TreeMap = std::map<Key, Type>;

template <typename Type>
using HashSet = std::unordered_set<Type>;

template <typename Type>
using TreeSet = std::set<Type>;

}