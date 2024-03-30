#pragma once

namespace aka {

enum class AllocatorFlags : uint8_t {
	None,
};

enum class AllocatorMemoryType : uint8_t {
	Temporary,
	Persistent,

	First = Temporary,
	Last = Persistent,
};

enum class AllocatorCategory : uint8_t {
	Default,
	Graphic,
	String,
	Vector,
	Pool,
	List,
	Component,

	First = Default,
	Last = Component,
};


};