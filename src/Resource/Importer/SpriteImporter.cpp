#include <Aka/Resource/Importer/SpriteImporter.h>

#include <Aka/OS/Image.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/OS/Archive.h>
#include <Aka/Resource/Asset.h>
#include <Aka/Resource/Resource/Sprite.h>
//#include <Aka/Resource/Archive/SpriteArchive.h>
#include <Aka/OS/OS.h>

#include <miniz.h>

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
		String name;
		Word from, to;
		LoopAnimationDirection direction;
		Byte rgb[3];
	};

	struct UserData {
		String string;
		Byte color[4];
	};

	struct Slice : UserData {

	};

	struct Cel : UserData {
		Word layerID;
		Short x, y;
		Byte opacity;
		Word width, height;
		Vector<Color32> pixels;
	};

	struct Layer : UserData {
		String name;
		LayerType type;
		LayerBlendMode blendMode;
		LayerFlags flags;
		Word childLevel;
		Byte opacity;

		bool hasFlag(LayerFlags flag) const;
	};

	struct Frame {
		Word duration;
		Vector<Cel> cels;

		// Generate an image from all the cells
		Vector<Color32> image(const Aseprite& ase) const;
	};

	Word width;
	Word height;
	ColorDepth colorDepth;
	DWord flags; // 1 = layer opacity has valid value
	Word speed; // deprecated
	Vector<Frame> frames;
	Vector<Layer> layers;
	Vector<Tag> tags;
	Vector<Slice> slices;
	Vector<uint8_t> palette;

	using BlendFunc = std::function<Color32(const Color32& src, const Color32& dst, Byte opacity)>;

	static uint8_t depth(ColorDepth colorDepth);

	static BlendFunc blending(LayerBlendMode blendMode);

	static Aseprite parse(Stream& reader);
};


bool Aseprite::Layer::hasFlag(LayerFlags flag) const
{
	return ((int)flag & (int)this->flags) == (int)flag;
}

Vector<Aseprite::Color32> Aseprite::Frame::image(const Aseprite& ase) const
{
	Vector<Color32> pixels(ase.width * ase.height, Color32{ 0 });
	for (const Cel& cel : cels)
	{
		const Layer& layer = ase.layers[cel.layerID];
		if (!layer.hasFlag(LayerFlags::Visible))
			continue;
		Word opacity = (Word)((cel.opacity / 255.f) * (layer.opacity / 255.f) * 255);
		BlendFunc blend = Aseprite::blending(layer.blendMode);

		for (Word yCel = 0; yCel < cel.height; yCel++)
		{
			for (Word xCel = 0; xCel < cel.width; xCel++)
			{
				if ((Short)yCel + cel.y < 0) continue;
				if ((Short)xCel + cel.x < 0) continue;
				if ((Short)yCel + cel.y >= ase.height) continue;
				if ((Short)xCel + cel.x >= ase.width) continue;
				Word index = cel.x + xCel + (cel.y + yCel) * ase.width;
				Word celIndex = xCel + yCel * cel.width;
				pixels[index] = blend(cel.pixels[celIndex], pixels[index], cel.opacity);
			}
		}
	}
	return pixels;
}

Aseprite Aseprite::parse(Stream& reader)
{
	BinaryArchive archive(reader);
	Aseprite ase;
	{
		// Parse header
		Aseprite::DWord fileSize = archive.read<Aseprite::DWord>();
		Aseprite::Word magicNumber = archive.read<Aseprite::Word>();
		if (magicNumber != 0xA5E0)
			throw std::runtime_error("File is not an aseprite file.");
		ase.frames.resize(archive.read<Aseprite::Word>());
		ase.width = archive.read<Aseprite::Word>();
		ase.height = archive.read<Aseprite::Word>();
		ase.colorDepth = archive.read<Aseprite::ColorDepth>();
		ase.flags = archive.read<Aseprite::DWord>();
		ase.speed = archive.read<Aseprite::Word>();
		Aseprite::DWord zero = archive.read<Aseprite::DWord>();
		AKA_ASSERT(zero == 0, "Should be zero");
		zero = archive.read<Aseprite::DWord>();
		AKA_ASSERT(zero == 0, "Should be zero");
		Aseprite::Byte paletteEntryIndex = archive.read<Aseprite::Byte>();
		archive.skim(3); // ignore
		Aseprite::Word colorCount = archive.read<Aseprite::Word>();
		Aseprite::Byte pixelWidth = archive.read<Aseprite::Byte>();
		Aseprite::Byte pixelHeight = archive.read<Aseprite::Byte>();
		float ratio = 1.f; // 1:1
		if (pixelWidth != 0 && pixelHeight != 0)
			ratio = pixelWidth / (float)pixelHeight;
		Aseprite::Short xGridPosition = archive.read<Aseprite::Short>();
		Aseprite::Short yGridPosition = archive.read<Aseprite::Short>();
		Aseprite::Word gridWidth = archive.read<Aseprite::Word>(); // zero if no grid
		Aseprite::Word gridHeight = archive.read<Aseprite::Word>(); // zero if no grid
		archive.skim(84); // reserved
	}

	{
		// Parse frame
		for (Aseprite::Frame& frame : ase.frames)
		{
			size_t frameOffset = archive.offset();
			// header
			Aseprite::DWord frameSize = archive.read<Aseprite::DWord>();
			Aseprite::Word magicNumber = archive.read<Aseprite::Word>();
			if (magicNumber != 0xF1FA)
				throw std::runtime_error("Incorrect frame magic number.");
			Aseprite::Word oldChunkCount = archive.read<Aseprite::Word>();
			frame.duration = archive.read<Aseprite::Word>();
			if (frame.duration == 0)
				frame.duration = ase.speed;
			archive.skim(2);
			Aseprite::DWord chunkCount = archive.read<Aseprite::DWord>();
			if (chunkCount == 0)
			{
				AKA_ASSERT(oldChunkCount != 0xFFFF, "Should be set to this value");
				chunkCount = oldChunkCount;
			}
			Aseprite::UserData* last = nullptr;
			// Parse chunks
			for (Aseprite::DWord iChunk = 0; iChunk < chunkCount; iChunk++)
			{
				size_t chunkOffset = archive.offset();
				size_t chunkHeaderSize = sizeof(Aseprite::DWord) + sizeof(Aseprite::ChunkType);
				Aseprite::DWord chunkSize = archive.read<Aseprite::DWord>();
				Aseprite::ChunkType chunkType = archive.read<Aseprite::ChunkType>();
				switch (chunkType)
				{
				case Aseprite::ChunkType::OldPaletteChunk: {
					Aseprite::Word packetCount = archive.read<Aseprite::Word>();
					for (Aseprite::Word iPacket = 0; iPacket < packetCount; iPacket++)
					{
						Aseprite::Byte packetPaletteOffset = archive.read<Aseprite::Byte>();
						Aseprite::Byte packetColorCount = archive.read<Aseprite::Byte>();
						for (Aseprite::Byte iColor = 0; iColor < packetColorCount; iColor++)
						{
							// 0-255
							Aseprite::Byte red = archive.read<Aseprite::Byte>();
							Aseprite::Byte green = archive.read<Aseprite::Byte>();
							Aseprite::Byte blue = archive.read<Aseprite::Byte>();
						}
					}
					break;
				}
				case Aseprite::ChunkType::OldPaletteChunk2: {
					Aseprite::Word packetCount = archive.read<Aseprite::Word>();
					for (Aseprite::Word iPacket = 0; iPacket < packetCount; iPacket++)
					{
						Aseprite::Byte packetPaletteOffset = archive.read<Aseprite::Byte>();
						Aseprite::Byte packetColorCount = archive.read<Aseprite::Byte>();
						for (Aseprite::Byte iColor = 0; iColor < packetColorCount; iColor++)
						{
							// 0-63
							Aseprite::Byte red = archive.read<Aseprite::Byte>();
							Aseprite::Byte green = archive.read<Aseprite::Byte>();
							Aseprite::Byte blue = archive.read<Aseprite::Byte>();
						}
					}
					break;
				}
				case Aseprite::ChunkType::LayerChunk: {
					Aseprite::Layer& layer = ase.layers.emplace();
					layer.flags = archive.read<Aseprite::LayerFlags>();
					layer.type = archive.read<Aseprite::LayerType>();
					layer.childLevel = archive.read<Aseprite::Word>();
					Aseprite::Word defaultLayerWidth = archive.read<Aseprite::Word>(); // ignored
					Aseprite::Word defaultLayerHeight = archive.read<Aseprite::Word>(); // ignored
					layer.blendMode = archive.read<Aseprite::LayerBlendMode>();
					layer.opacity = archive.read<Aseprite::Byte>();
					archive.skim(3);
					{
						Aseprite::Word stringLength = archive.read<Aseprite::Word>();
						layer.name.resize(stringLength);
						archive.read<char>(layer.name.data(), layer.name.size()); // UTF-8
					}
					last = &layer;
					break;
				}
				case Aseprite::ChunkType::CelChunk: {
					Aseprite::Cel& cel = frame.cels.emplace();
					cel.layerID = archive.read<Aseprite::Word>();
					cel.x = archive.read<Aseprite::Short>();
					cel.y = archive.read<Aseprite::Short>();
					cel.opacity = archive.read<Aseprite::Byte>();
					Aseprite::Word celType = archive.read<Aseprite::Word>();
					archive.skim(7);

					std::vector<uint8_t> pixels;
					if (celType == 0)
					{
						// raw cel
						cel.width = archive.read<Aseprite::Word>();
						cel.height = archive.read<Aseprite::Word>();
						pixels.resize(cel.width * cel.height * Aseprite::depth(ase.colorDepth));
						// Indexed, grayscale or rgba depending on ase.depth
						// Stored row by row from top to bottom
						archive.read(pixels.data(), pixels.size());
					}
					else if (celType == 1)
					{
						// linked cell
						Aseprite::Word framePosition = archive.read<Aseprite::Word>();
					}
					else if (celType == 2)
					{
						// compressed image
						cel.width = archive.read<Aseprite::Word>();
						cel.height = archive.read<Aseprite::Word>();
						pixels.resize(cel.width * cel.height * Aseprite::depth(ase.colorDepth));
						{
							// Deflate data
							mz_ulong size = (mz_ulong)pixels.size();
							mz_ulong deflateBytes = (mz_ulong)(chunkSize - (archive.offset() - chunkOffset));
							std::vector<uint8_t> bytes(deflateBytes);
							archive.read(bytes.data(), bytes.size());
							if (MZ_OK != mz_uncompress(pixels.data(), &size, bytes.data(), deflateBytes))
								throw std::runtime_error("Failed to uncompress image.");
						}
					}
					{
						// Convert uint8 to color32
						cel.pixels.resize(cel.width * cel.height);
						switch (ase.colorDepth)
						{
						case Aseprite::ColorDepth::Rgba: {
							for (size_t iPixel = 0; iPixel < cel.pixels.size(); iPixel++)
							{
								cel.pixels[iPixel].r = pixels[iPixel * 4 + 0];
								cel.pixels[iPixel].g = pixels[iPixel * 4 + 1];
								cel.pixels[iPixel].b = pixels[iPixel * 4 + 2];
								cel.pixels[iPixel].a = pixels[iPixel * 4 + 3];
							}
							break;
						}
						case Aseprite::ColorDepth::GrayScale: {
							for (size_t iPixel = 0; iPixel < cel.pixels.size(); iPixel++)
							{
								cel.pixels[iPixel].r = pixels[iPixel * 2 + 0];
								cel.pixels[iPixel].g = pixels[iPixel * 2 + 0];
								cel.pixels[iPixel].b = pixels[iPixel * 2 + 0];
								cel.pixels[iPixel].a = pixels[iPixel * 2 + 1];
							}
							break;
						}
						case Aseprite::ColorDepth::Indexed: {
							for (size_t iPixel = 0; iPixel < cel.pixels.size(); iPixel++)
							{
								cel.pixels[iPixel].r = ase.palette[iPixel * 4 + 0];
								cel.pixels[iPixel].g = ase.palette[iPixel * 4 + 1];
								cel.pixels[iPixel].b = ase.palette[iPixel * 4 + 2];
								cel.pixels[iPixel].a = ase.palette[iPixel * 4 + 3];
							}
							break;
						}
						}
					}
					last = &cel;
					break;
				}
				case Aseprite::ChunkType::CelExtraChunk: {
					Aseprite::DWord flags = archive.read<Aseprite::DWord>(); // 1 = precise bounds are set
					Aseprite::Fixed preciseX = archive.read<Aseprite::Fixed>();
					Aseprite::Fixed preciseY = archive.read<Aseprite::Fixed>();
					Aseprite::Fixed celWidthInSprite = archive.read<Aseprite::Fixed>();
					Aseprite::Fixed celHeightInSprite = archive.read<Aseprite::Fixed>();
					archive.skim(16);
					break;
				}
				case Aseprite::ChunkType::ColorProfileChunk: {
					Aseprite::Word type = archive.read<Aseprite::Word>(); // 0 -> no color profile, 1 -> sRGB, 2 -> embedded ICC
					Aseprite::Word flags = archive.read<Aseprite::Word>(); // 1 -> use special fixed gamma
					Aseprite::Fixed gamma = archive.read<Aseprite::Fixed>(); // 1.0 -> linear
					archive.skim(8); // reserved
					if ((flags & 0x02) == 0x02)
					{
						Aseprite::DWord iccProfileLength = archive.read<Aseprite::DWord>();
						std::vector<Aseprite::Byte> iccProfileData(iccProfileLength);
						archive.read(iccProfileData.data(), iccProfileData.size());
					}
					break;
				}
				case Aseprite::ChunkType::MaskChunk: {
					Aseprite::Short xPosition = archive.read<Aseprite::Short>();
					Aseprite::Short yPosition = archive.read<Aseprite::Short>();
					Aseprite::Word width = archive.read<Aseprite::Word>();
					Aseprite::Word height = archive.read<Aseprite::Word>();
					archive.skim(8);
					std::string maskName;
					{
						Aseprite::Word stringLength = archive.read<Aseprite::Word>();
						maskName.resize(stringLength);
						archive.read<char>(maskName.data(), maskName.size()); // UTF-8
					}
					std::vector<Aseprite::Byte> bytes(height * ((width + 7) / 8));
					archive.read(bytes.data(), bytes.size());
					break;
				}
				case Aseprite::ChunkType::PathChunk: {
					archive.skim(chunkSize - chunkHeaderSize);
					break;
				}
				case Aseprite::ChunkType::TagsChunk: {
					Aseprite::Word tagCount = archive.read<Aseprite::Word>();
					size_t tagOffset = ase.tags.size();
					ase.tags.resize(tagOffset + tagCount);
					archive.skim(8);
					for (Aseprite::Word iTag = 0; iTag < tagCount; iTag++)
					{
						Aseprite::Tag& tag = ase.tags[tagOffset + iTag];
						tag.from = archive.read<Aseprite::Word>();
						tag.to = archive.read<Aseprite::Word>();
						tag.direction = archive.read<Aseprite::LoopAnimationDirection>(); // 0 ->forward, 1 -> reverse, 2 -> ping pong
						archive.skim(8);
						tag.rgb[0] = archive.read<Aseprite::Byte>();
						tag.rgb[1] = archive.read<Aseprite::Byte>();
						tag.rgb[2] = archive.read<Aseprite::Byte>();
						archive.skim(1);
						{
							Aseprite::Word stringLength = archive.read<Aseprite::Word>();
							tag.name.resize(stringLength);
							archive.read<char>(tag.name.data(), tag.name.size()); // UTF-8
						}
					}
					break;
				}
				case Aseprite::ChunkType::PaletteChunk: {
					Aseprite::DWord paletteSize = archive.read<Aseprite::DWord>();
					Aseprite::DWord firstColor = archive.read<Aseprite::DWord>();
					Aseprite::DWord lastColor = archive.read<Aseprite::DWord>();
					AKA_ASSERT(lastColor - firstColor + 1 == paletteSize, "Incorrect palette count");
					ase.palette.resize(ase.palette.size() + paletteSize * 4);
					archive.skim(8);
					for (Aseprite::DWord iEntry = firstColor; iEntry <= lastColor; iEntry++)
					{
						Aseprite::Word entryFlags = archive.read<Aseprite::Word>(); // 1 = HasName
						ase.palette[firstColor * 4 + iEntry * 4 + 0] = archive.read<Aseprite::Byte>();
						ase.palette[firstColor * 4 + iEntry * 4 + 1] = archive.read<Aseprite::Byte>();
						ase.palette[firstColor * 4 + iEntry * 4 + 2] = archive.read<Aseprite::Byte>();
						ase.palette[firstColor * 4 + iEntry * 4 + 3] = archive.read<Aseprite::Byte>();
						std::string colorName = "none";
						if ((entryFlags & 0x01) == 0x01)
						{
							Aseprite::Word stringLength = archive.read<Aseprite::Word>();
							colorName.resize(stringLength);
							archive.read<char>(colorName.data(), colorName.size()); // UTF-8
						}
					}
					break;
				}
				case Aseprite::ChunkType::UserDataChunk: {
					if (last != nullptr)
					{
						Aseprite::DWord flags = archive.read<Aseprite::DWord>();
						if ((flags & 0x01) == 0x01)
						{
							Aseprite::Word stringLength = archive.read<Aseprite::Word>();
							last->string.resize(stringLength);
							archive.read<char>(last->string.data(), last->string.size()); // UTF-8
						}
						if ((flags & 0x02) == 0x02)
						{
							last->color[0] = archive.read<Aseprite::Byte>();
							last->color[1] = archive.read<Aseprite::Byte>();
							last->color[2] = archive.read<Aseprite::Byte>();
							last->color[3] = archive.read<Aseprite::Byte>();
						}
					}
					else
					{
						archive.skim(chunkSize - chunkHeaderSize);
					}
					break;
				}
				case Aseprite::ChunkType::SliceChunk: {
					// Slice chunk
					Aseprite::DWord sliceKeyCount = archive.read<Aseprite::DWord>();
					Aseprite::DWord flags = archive.read<Aseprite::DWord>(); // 1 -> 9 patch slice, 2 -> has pivot information
					Aseprite::DWord reserved = archive.read<Aseprite::DWord>();
					std::string slicesName;
					{
						Aseprite::Word stringLength = archive.read<Aseprite::Word>();
						slicesName.resize(stringLength);
						archive.read<char>(slicesName.data(), slicesName.size()); // UTF-8
					}
					for (Aseprite::DWord iSliceKey = 0; iSliceKey < sliceKeyCount; iSliceKey++)
					{
						Aseprite::DWord frameNumber = archive.read<Aseprite::DWord>();
						Aseprite::Long xOrigin = archive.read<Aseprite::Long>();
						Aseprite::Long yOrigin = archive.read<Aseprite::Long>();
						Aseprite::DWord width = archive.read<Aseprite::DWord>(); // can be zero if hidden in the anim from the given frame)
						Aseprite::DWord height = archive.read<Aseprite::DWord>();
						if ((flags & 0x01) == 0x01)
						{
							Aseprite::Long centerX = archive.read<Aseprite::Long>();
							Aseprite::Long centerY = archive.read<Aseprite::Long>();
							Aseprite::DWord centerWidth = archive.read<Aseprite::DWord>();
							Aseprite::DWord centerHeight = archive.read<Aseprite::DWord>();
						}
						if ((flags & 0x02) == 0x02)
						{
							Aseprite::Long pivotXPosition = archive.read<Aseprite::Long>();
							Aseprite::Long pivotYPosition = archive.read<Aseprite::Long>();
						}
					}
					break;
				}
				default: {
					Logger::warn("Unsupported Aseprite chunk type. Skipping");
					archive.skim(chunkSize - chunkHeaderSize);
				}
				}
				AKA_ASSERT(archive.offset() - chunkOffset == chunkSize, "Invalid chunk reading");
			}
			AKA_ASSERT(archive.offset() - frameOffset == frameSize, "Invalid frame reading");
		}
	}
	return ase;
}

uint8_t Aseprite::depth(ColorDepth colorDepth)
{
	switch (colorDepth) {
	case ColorDepth::Indexed:
		return 1;
	case ColorDepth::GrayScale:
		return 2;
	case ColorDepth::Rgba:
		return 4;
	default:
		return 0;
	}
}

// From pixman-combine32.h
#define MUL_UN8(a, b, t) ((t) = ((int)a) * ((int)b) + 0x80, (((((int)t) >> 8 ) + ((int)t) ) >> 8 ))
// https://github.com/aseprite/aseprite/blob/master/src/doc/blend_funcs.cpp
Aseprite::BlendFunc Aseprite::blending(LayerBlendMode blendMode)
{
	switch (blendMode)
	{
	default:
		Logger::warn("Blending mode not implementing. Normal blending fallback");
	case Aseprite::LayerBlendMode::Normal:
		return [](const Color32& src, const Color32& dst, Aseprite::Byte opacity) -> Color32
		{
			int r, g, b, a;
			int tmp;
			if (dst.a == 0)
			{
				r = src.r;
				g = src.g;
				b = src.b;
			}
			else if (src.a == 0)
			{
				r = dst.r;
				g = dst.g;
				b = dst.b;
			}
			else
			{
				r = (dst.r + MUL_UN8((src.r - dst.r), opacity, tmp));
				g = (dst.g + MUL_UN8((src.g - dst.g), opacity, tmp));
				b = (dst.b + MUL_UN8((src.b - dst.b), opacity, tmp));
			}
			a = (dst.a + MUL_UN8((src.a - dst.a), opacity, tmp));
			if (a == 0)
				return Color32{};
			return Color32{ (Byte)r,(Byte)g,(Byte)b,(Byte)a };
		};
	case Aseprite::LayerBlendMode::Multiply:
		return [](const Color32& src, const Color32& dst, Aseprite::Byte opacity) -> Color32
		{
			int tmp;
			return blending(Aseprite::LayerBlendMode::Normal)(
				Color32{
					(Aseprite::Byte)MUL_UN8(dst.r, src.r, tmp),
					(Aseprite::Byte)MUL_UN8(dst.g, src.g, tmp),
					(Aseprite::Byte)MUL_UN8(dst.b, src.b, tmp),
					0
				},
				dst,
				opacity
				);
		};
	}
}



bool SpriteImporter::import(const Path& path, std::function<bool(Resource* resource)>&& callback)
{
	FileStream stream(path, FileMode::Read, FileType::Binary);
	Aseprite ase = Aseprite::parse(stream);

	Sprite* sprite = new Sprite;
	sprite->createBuildData();
	SpriteBuildData* buildData = reinterpret_cast<SpriteBuildData*>(sprite->getBuildData());

	auto convertFrame = [](const Aseprite& ase, const Aseprite::Frame& aseFrame) -> SpriteBuildData::Frame
	{
		Vector<Aseprite::Color32> aseImage = aseFrame.image(ase);
		Image image(ase.width, ase.height, ImageComponent::RGBA);
		memcpy(image.data(), aseImage[0].data, image.size());
		// Set frame
		SpriteBuildData::Frame frame;
		frame.duration = Time::milliseconds(aseFrame.duration);
		frame.bytes = Blob(image.data(), image.size());
		return frame;
	};

	if (ase.tags.size() == 0)
	{
		// Default animation
		SpriteBuildData::Animation animation;
		animation.name = "Idle";
		for (Aseprite::Frame& aseFrame : ase.frames)
		{
			buildData->width = min<uint32_t>(buildData->width, ase.width);
			buildData->height = min<uint32_t>(buildData->height, ase.height);
			animation.frames.append(convertFrame(ase, aseFrame));
		}
		buildData->animations.append(animation);
	}
	else
	{
		for (Aseprite::Tag& aseTag : ase.tags)
		{
			SpriteBuildData::Animation animation;
			animation.name = aseTag.name;
			for (Aseprite::Word iFrame = aseTag.from; iFrame <= aseTag.to; iFrame++)
			{
				buildData->width = min<uint32_t>(buildData->width, ase.width);
				buildData->height = min<uint32_t>(buildData->height, ase.height);
				animation.frames.append(convertFrame(ase, ase.frames[iFrame]));
			}
			buildData->animations.append(animation);
		}
	}
	return callback(sprite);
}

};