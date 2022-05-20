#pragma once

#include <Aka/OS/Path.h>
#include <Aka/Resource/Asset.h>

#include <map>
#include <functional>

namespace aka {

struct Asset;


class AssetRegistry
{
public:
	// Parse registry from file
	void parse(const Path& path);
	// Serialize registry to file
	void serialize(const Path& path);

	// Find an asset from registry
	Asset find(const char* name);
	// Add an asset to registry
	void add(const char* name, Asset asset);
	// Remove an asset
	void remove(const char* name);

	//template <typename T>
	//void forEach(std::function<void(T* resource)>&& callback) {}

	// Iterator
	struct Iterator
	{
		using IteratorType = std::map<String, Asset>::iterator;
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = Asset;
		using pointer = Asset*;  // or also value_type*
		using reference = Asset&;  // or also value_type&

		Iterator(IteratorType ptr) : m_iterator(ptr) {}

		reference operator*() const { return m_iterator->second; }
		pointer operator->() { return &m_iterator->second; }
		Iterator& operator++() { m_iterator++; return *this; }
		Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

		friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_iterator->first == b.m_iterator->first; };
		friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_iterator->first != b.m_iterator->first; };
	private:
		IteratorType m_iterator;
	};

	struct ConstIterator
	{
		using IteratorType = std::map<String, Asset>::const_iterator;
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = Asset;
		using pointer = const Asset*;  // or also value_type*
		using reference = const Asset&;  // or also value_type&

		ConstIterator(IteratorType ptr) : m_iterator(ptr) {}

		reference operator*() const { return m_iterator->second; }
		pointer operator->() { return &m_iterator->second; }
		ConstIterator& operator++() { m_iterator++; return *this; }
		ConstIterator operator++(int) { ConstIterator tmp = *this; ++(*this); return tmp; }

		friend bool operator== (const ConstIterator& a, const ConstIterator& b) { return a.m_iterator->first == b.m_iterator->first; };
		friend bool operator!= (const ConstIterator& a, const ConstIterator& b) { return a.m_iterator->first != b.m_iterator->first; };
	private:
		IteratorType m_iterator;
	};
	Iterator begin() { return Iterator(m_assets.begin()); };
	Iterator end() { return Iterator(m_assets.begin()); };
	ConstIterator begin() const { return ConstIterator(m_assets.begin()); };
	ConstIterator end() const { return ConstIterator(m_assets.begin()); };
private:
	std::map<String, Asset> m_assets;
};

};