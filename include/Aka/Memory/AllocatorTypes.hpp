#pragma once

namespace aka {

enum class AllocatorFlags {
	None,
};

enum class AllocatorMemoryType {
	Temporary,
	Persistent,

	First = Temporary,
	Last = Persistent,
};

enum class AllocatorCategory {
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