#pragma once

#include <vector>
#include <string>
#include <functional>

#include <Aka/OS/Stream/Stream.h>

namespace aka {

// Ase sprite parser 
// This does not support every features of the file format
// Complete aseprite spec
// https://github.com/aseprite/aseprite/blob/master/docs/ase-file-specs.md
// Helped by Noël Berry's work
// https://gist.github.com/NoelFB/778d190e5d17f1b86ebf39325346fcc5
struct Aseprite {
	using Byte = uint8_t;
	using Word = uint16_t;
	using Short = int16_t;
	using DWord = uint32_t;
	using Long = int32_t;
	using Fixed = int32_t; // two 16 ?

	enum class ColorDepth : Word {
		Indexed = 8,
		GrayScale = 16,
		Rgba = 32,
	};

	enum class LayerFlags : Word {
		Visible = 1,
		Editable = 2,
		LockMovement = 4,
		Background = 8,
		PreferLinkedCels = 16,
		LayerGroupCollapsed = 32,
		ReferenceLayer = 64,
	};

	enum class LayerType : Word {
		Normal = 0,
		Group = 1,
	};

	enum class LayerBlendMode : Word {
		Normal = 0,
		Multiply = 1,
		Screen = 2,
		Overlay = 3,
		Darken = 4,
		Lighten = 5,
		ColorDodge = 6,
		ColorBurn = 7,
		HardLight = 8,
		SoftLight = 9,
		Difference = 10,
		Exclusion = 11,
		Hue = 12,
		Saturation = 13,
		Color = 14,
		Luminosity = 15,
		Addition = 16,
		Subtract = 17,
		Divide = 18,
	};

	enum class ChunkType : Word {
		OldPaletteChunk = 0x0004, // Ignore if PaletteChunk set
		OldPaletteChunk2 = 0x0011, // Ignore if PaletteChunk set
		LayerChunk = 0x2004,
		CelChunk = 0x2005,
		CelExtraChunk = 0x2006,
		ColorProfileChunk = 0x2007,
		MaskChunk = 0x2016, // Deprecated
		PathChunk = 0x2017, // Never used
		TagsChunk = 0x2018,
		PaletteChunk = 0x2019,
		UserDataChunk = 0x2020,
		SliceChunk = 0x2022,
	};

	enum class LoopAnimationDirection : Byte {
		Forward,
		Reverse,
		PingPong
	};

	struct Color32 {
		union {
			Byte data[4];
			struct {
				Byte r, g, b, a;
			};
		};
	};

	struct Tag {
		std::string name;
		Word from, to;
		LoopAnimationDirection direction;
		Byte rgb[3];
	};

	struct UserData {
		std::string string;
		Byte color[4];
	};

	struct Slice : UserData {

	};

	struct Cel : UserData {
		Word layerID;
		Short x, y;
		Byte opacity;
		Word width, height;
		std::vector<Color32> pixels;
	};

	struct Layer : UserData {
		std::string name;
		LayerType type;
		LayerBlendMode blendMode;
		LayerFlags flags;
		Word childLevel;
		Byte opacity;

		bool hasFlag(LayerFlags flag) const;
	};

	struct Frame {
		Word duration;
		std::vector<Cel> cels;

		// Generate an image from all the cells
		std::vector<Color32> image(const Aseprite &ase) const;
	};

	Word width;
	Word height;
	ColorDepth colorDepth;
	DWord flags; // 1 = layer opacity has valid value
	Word speed; // deprecated
	std::vector<Frame> frames;
	std::vector<Layer> layers;
	std::vector<Tag> tags;
	std::vector<Slice> slices;
	std::vector<uint8_t> palette;

	using BlendFunc = std::function<Color32(const Color32& src, const Color32& dst, Byte opacity)>;

	static uint8_t depth(ColorDepth colorDepth);

	static BlendFunc blending(LayerBlendMode blendMode);

	static Aseprite parse(Stream& reader);
};

};