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
	RawArray, // untracked new[]


	First = Vector,
	Last = RawArray,
};

enum class AllocatorCategory : uint8_t
{
	Global,
	Platform,
	Graphic,
	Audio,
	Assets,
	Component,
	Editor,

	First = Global,
	Last = Editor,
};

};