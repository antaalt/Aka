#pragma once

namespace aka {

enum class AllocatorFlags : uint8_t {
	None,
};

enum class AllocatorMemoryType : uint8_t {
	Temporary,
	Persistent,
	// Could be Malloc, Pool...

	First = Temporary,
	Last = Persistent,
};

enum class AllocatorCategory : uint8_t {
	Default,
	Graphic,
	Audio,
	String,
	Vector,
	Pool,
	List,
	Component,
	Archive,
	Editor,

	First = Default,
	Last = Editor,
};

};