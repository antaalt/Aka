#pragma once

namespace aka {

enum class AllocatorFlags : uint8_t
{
	None,
};

enum class AllocatorMemoryType : uint8_t
{
	Vector,
	Map,
	Blob,
	Set,
	List,
	String,
	Pool,
	Object,
	Raw, // untracked new (malloc not tracked)


	First = Vector,
	Last = Raw,
};

enum class AllocatorCategory : uint8_t
{
	Global,
	Graphic,
	Audio,
	Assets,
	Component,
	Editor,

	First = Global,
	Last = Editor,
};

};