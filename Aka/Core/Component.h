#pragma once

#include <stdint.h>

namespace aka {

class World;
struct Entity;

// IAComponent
// ColliderComponent
// TransformComponent
// GravityComponent
struct Component
{
public:
	friend class World;
	struct Type
	{
		// Get the id for the specified type
		template <typename T>
		static uint8_t get() {
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
public:
	uint8_t getType() const;
	const Entity* getEntity() const;
private:
	uint8_t m_type;
	Entity* m_entity;
};

};