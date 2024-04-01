#pragma once

#include <Aka/Core/Container/Vector.h>

namespace aka {

struct BitField
{
	BitField();
	BitField(uint32_t bitCount);

	bool operator[](size_t index) const;

	void set(size_t index);
	void set(size_t index, bool value);
	void clear(size_t index);
	void toggle(size_t index);
	bool check(size_t index) const;

	void resize(size_t bitCount);
	void reserve(size_t bitCount);

	void clear();
private:
	Vector<uint8_t> m_container;
};

};