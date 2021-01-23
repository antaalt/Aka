#pragma once

#include "../Debug.h"

#include <stdint.h>
#include <map>
#include <vector>

namespace aka {

class World;

struct Event
{
	virtual ~Event() {}
	struct Type
	{
		// Get the id for the specified type
		template <typename T>
		static uint8_t get() {
			ASSERT(m_typeCounter < 255, "Reach max type capacity");
			static const uint8_t type = m_typeCounter++;
			return type;
		}
		// Current number of different type
		static uint8_t count() {
			return m_typeCounter;
		}
		// Maximum number of different type
		static constexpr uint8_t size() { return 255; }
	private:
		static inline uint8_t m_typeCounter = 0;
	};
};

};