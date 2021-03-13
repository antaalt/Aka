#include <Aka/Core/Aseprite.h>

#include <Aka/OS/Logger.h>
#include <Aka/Core/Debug.h>
#include <Aka/Core/Geometry.h>

#include <miniz.h>

namespace aka {

bool Aseprite::Layer::hasFlag(LayerFlags flag) const
{
	return ((int)flag & (int)this->flags) == (int)flag;
}

std::vector<Aseprite::Color32> Aseprite::Frame::image(const Aseprite& ase) const
{
	std::vector<Color32> pixels(ase.width * ase.height, Color32{ 0 });
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
	Aseprite ase;
	{
		// Parse header
		Aseprite::DWord fileSize = reader.read<Aseprite::DWord>();
		Aseprite::Word magicNumber = reader.read<Aseprite::Word>();
		if (magicNumber != 0xA5E0)
			throw std::runtime_error("File is not an aseprite file.");
		ase.frames.resize(reader.read<Aseprite::Word>());
		ase.width = reader.read<Aseprite::Word>();
		ase.height = reader.read<Aseprite::Word>();
		ase.colorDepth = reader.read<Aseprite::ColorDepth>();
		ase.flags = reader.read<Aseprite::DWord>();
		ase.speed = reader.read<Aseprite::Word>();
		Aseprite::DWord zero = reader.read<Aseprite::DWord>();
		ASSERT(zero == 0, "Should be zero");
		zero = reader.read<Aseprite::DWord>();
		ASSERT(zero == 0, "Should be zero");
		Aseprite::Byte paletteEntryIndex = reader.read<Aseprite::Byte>();
		reader.skim(3); // ignore
		Aseprite::Word colorCount = reader.read<Aseprite::Word>();
		Aseprite::Byte pixelWidth = reader.read<Aseprite::Byte>();
		Aseprite::Byte pixelHeight = reader.read<Aseprite::Byte>();
		float ratio = 1.f; // 1:1
		if (pixelWidth != 0 && pixelHeight != 0)
			ratio = pixelWidth / (float)pixelHeight;
		Aseprite::Short xGridPosition = reader.read<Aseprite::Short>();
		Aseprite::Short yGridPosition = reader.read<Aseprite::Short>();
		Aseprite::Word gridWidth = reader.read<Aseprite::Word>(); // zero if no grid
		Aseprite::Word gridHeight = reader.read<Aseprite::Word>(); // zero if no grid
		reader.skim(84); // reserved
	}

	{
		// Parse frame
		for (Aseprite::Frame& frame : ase.frames)
		{
			size_t frameOffset = reader.offset();
			// header
			Aseprite::DWord frameSize = reader.read<Aseprite::DWord>();
			Aseprite::Word magicNumber = reader.read<Aseprite::Word>();
			if (magicNumber != 0xF1FA)
				throw std::runtime_error("Incorrect frame magic number.");
			Aseprite::Word oldChunkCount = reader.read<Aseprite::Word>();
			frame.duration = reader.read<Aseprite::Word>();
			if (frame.duration == 0)
				frame.duration = ase.speed;
			reader.skim(2);
			Aseprite::DWord chunkCount = reader.read<Aseprite::DWord>();
			if (chunkCount == 0)
			{
				ASSERT(oldChunkCount != 0xFFFF, "Should be set to this value");
				chunkCount = oldChunkCount;
			}
			Aseprite::UserData* last = nullptr;
			// Parse chunks
			for (Aseprite::DWord iChunk = 0; iChunk < chunkCount; iChunk++)
			{
				size_t chunkOffset = reader.offset();
				size_t chunkHeaderSize = sizeof(Aseprite::DWord) + sizeof(Aseprite::ChunkType);
				Aseprite::DWord chunkSize = reader.read<Aseprite::DWord>();
				Aseprite::ChunkType chunkType = reader.read<Aseprite::ChunkType>();
				switch (chunkType)
				{
				case Aseprite::ChunkType::OldPaletteChunk: {
					Aseprite::Word packetCount = reader.read<Aseprite::Word>();
					for (Aseprite::Word iPacket = 0; iPacket < packetCount; iPacket++)
					{
						Aseprite::Byte packetPaletteOffset = reader.read<Aseprite::Byte>();
						Aseprite::Byte packetColorCount = reader.read<Aseprite::Byte>();
						for (Aseprite::Byte iColor = 0; iColor < packetColorCount; iColor++)
						{
							// 0-255
							Aseprite::Byte red = reader.read<Aseprite::Byte>();
							Aseprite::Byte green = reader.read<Aseprite::Byte>();
							Aseprite::Byte blue = reader.read<Aseprite::Byte>();
						}
					}
					break;
				}
				case Aseprite::ChunkType::OldPaletteChunk2: {
					Aseprite::Word packetCount = reader.read<Aseprite::Word>();
					for (Aseprite::Word iPacket = 0; iPacket < packetCount; iPacket++)
					{
						Aseprite::Byte packetPaletteOffset = reader.read<Aseprite::Byte>();
						Aseprite::Byte packetColorCount = reader.read<Aseprite::Byte>();
						for (Aseprite::Byte iColor = 0; iColor < packetColorCount; iColor++)
						{
							// 0-63
							Aseprite::Byte red = reader.read<Aseprite::Byte>();
							Aseprite::Byte green = reader.read<Aseprite::Byte>();
							Aseprite::Byte blue = reader.read<Aseprite::Byte>();
						}
					}
					break;
				}
				case Aseprite::ChunkType::LayerChunk: {
					ase.layers.emplace_back();
					Aseprite::Layer& layer = ase.layers.back();
					layer.flags = reader.read<Aseprite::LayerFlags>();
					layer.type = reader.read<Aseprite::LayerType>();
					layer.childLevel = reader.read<Aseprite::Word>();
					Aseprite::Word defaultLayerWidth = reader.read<Aseprite::Word>(); // ignored
					Aseprite::Word defaultLayerHeight = reader.read<Aseprite::Word>(); // ignored
					layer.blendMode = reader.read<Aseprite::LayerBlendMode>();
					layer.opacity = reader.read<Aseprite::Byte>();
					reader.skim(3);
					{
						Aseprite::Word stringLength = reader.read<Aseprite::Word>();
						layer.name.resize(stringLength);
						reader.read<char>(layer.name.data(), layer.name.size()); // UTF-8
					}
					last = &layer;
					break;
				}
				case Aseprite::ChunkType::CelChunk: {
					frame.cels.emplace_back();
					Aseprite::Cel& cel = frame.cels.back();
					cel.layerID = reader.read<Aseprite::Word>();
					cel.x = reader.read<Aseprite::Short>();
					cel.y = reader.read<Aseprite::Short>();
					cel.opacity = reader.read<Aseprite::Byte>();
					Aseprite::Word celType = reader.read<Aseprite::Word>();
					reader.skim(7);

					std::vector<uint8_t> pixels;
					if (celType == 0)
					{
						// raw cel
						cel.width = reader.read<Aseprite::Word>();
						cel.height = reader.read<Aseprite::Word>();
						pixels.resize(cel.width * cel.height * Aseprite::depth(ase.colorDepth));
						// Indexed, grayscale or rgba depending on ase.depth
						// Stored row by row from top to bottom
						reader.read(pixels.data(), pixels.size());
					}
					else if (celType == 1)
					{
						// linked cell
						Aseprite::Word framePosition = reader.read<Aseprite::Word>();
					}
					else if (celType == 2)
					{
						// compressed image
						cel.width = reader.read<Aseprite::Word>();
						cel.height = reader.read<Aseprite::Word>();
						pixels.resize(cel.width * cel.height * Aseprite::depth(ase.colorDepth));
						{
							// Deflate data
							mz_ulong size = (mz_ulong)pixels.size();
							mz_ulong deflateBytes = (mz_ulong)(chunkSize - (reader.offset() - chunkOffset));
							std::vector<uint8_t> bytes(deflateBytes);
							reader.read(bytes.data(), bytes.size());
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
					Aseprite::DWord flags = reader.read<Aseprite::DWord>(); // 1 = precise bounds are set
					Aseprite::Fixed preciseX = reader.read<Aseprite::Fixed>();
					Aseprite::Fixed preciseY = reader.read<Aseprite::Fixed>();
					Aseprite::Fixed celWidthInSprite = reader.read<Aseprite::Fixed>();
					Aseprite::Fixed celHeightInSprite = reader.read<Aseprite::Fixed>();
					reader.skim(16);
					break;
				}
				case Aseprite::ChunkType::ColorProfileChunk: {
					Aseprite::Word type = reader.read<Aseprite::Word>(); // 0 -> no color profile, 1 -> sRGB, 2 -> embedded ICC
					Aseprite::Word flags = reader.read<Aseprite::Word>(); // 1 -> use special fixed gamma
					Aseprite::Fixed gamma = reader.read<Aseprite::Fixed>(); // 1.0 -> linear
					reader.skim(8); // reserved
					if ((flags & 0x02) == 0x02)
					{
						Aseprite::DWord iccProfileLength = reader.read<Aseprite::DWord>();
						std::vector<Aseprite::Byte> iccProfileData(iccProfileLength);
						reader.read(iccProfileData.data(), iccProfileData.size());
					}
					break;
				}
				case Aseprite::ChunkType::MaskChunk: {
					Aseprite::Short xPosition = reader.read<Aseprite::Short>();
					Aseprite::Short yPosition = reader.read<Aseprite::Short>();
					Aseprite::Word width = reader.read<Aseprite::Word>();
					Aseprite::Word height = reader.read<Aseprite::Word>();
					reader.skim(8);
					std::string maskName;
					{
						Aseprite::Word stringLength = reader.read<Aseprite::Word>();
						maskName.resize(stringLength);
						reader.read<char>(maskName.data(), maskName.size()); // UTF-8
					}
					std::vector<Aseprite::Byte> bytes(height * ((width + 7) / 8));
					reader.read(bytes.data(), bytes.size());
					break;
				}
				case Aseprite::ChunkType::PathChunk: {
					reader.skim(chunkSize - chunkHeaderSize);
					break;
				}
				case Aseprite::ChunkType::TagsChunk: {
					Aseprite::Word tagCount = reader.read<Aseprite::Word>();
					size_t tagOffset = ase.tags.size();
					ase.tags.resize(tagOffset + tagCount);
					reader.skim(8);
					for (Aseprite::Word iTag = 0; iTag < tagCount; iTag++)
					{
						Aseprite::Tag& tag = ase.tags[tagOffset + iTag];
						tag.from = reader.read<Aseprite::Word>();
						tag.to = reader.read<Aseprite::Word>();
						tag.direction = reader.read<Aseprite::LoopAnimationDirection>(); // 0 ->forward, 1 -> reverse, 2 -> ping pong
						reader.skim(8);
						tag.rgb[0] = reader.read<Aseprite::Byte>();
						tag.rgb[1] = reader.read<Aseprite::Byte>();
						tag.rgb[2] = reader.read<Aseprite::Byte>();
						reader.skim(1);
						{
							Aseprite::Word stringLength = reader.read<Aseprite::Word>();
							tag.name.resize(stringLength);
							reader.read<char>(tag.name.data(), tag.name.size()); // UTF-8
						}
					}
					break;
				}
				case Aseprite::ChunkType::PaletteChunk: {
					Aseprite::DWord paletteSize = reader.read<Aseprite::DWord>();
					Aseprite::DWord firstColor = reader.read<Aseprite::DWord>();
					Aseprite::DWord lastColor = reader.read<Aseprite::DWord>();
					ASSERT(lastColor - firstColor + 1 == paletteSize, "Incorrect palette count");
					ase.palette.resize(ase.palette.size() + paletteSize * 4);
					reader.skim(8);
					for (Aseprite::DWord iEntry = firstColor; iEntry <= lastColor; iEntry++)
					{
						Aseprite::Word entryFlags = reader.read<Aseprite::Word>(); // 1 = HasName
						ase.palette[firstColor * 4 + iEntry * 4 + 0] = reader.read<Aseprite::Byte>();
						ase.palette[firstColor * 4 + iEntry * 4 + 1] = reader.read<Aseprite::Byte>();
						ase.palette[firstColor * 4 + iEntry * 4 + 2] = reader.read<Aseprite::Byte>();
						ase.palette[firstColor * 4 + iEntry * 4 + 3] = reader.read<Aseprite::Byte>();
						std::string colorName = "none";
						if ((entryFlags & 0x01) == 0x01)
						{
							Aseprite::Word stringLength = reader.read<Aseprite::Word>();
							colorName.resize(stringLength);
							reader.read<char>(colorName.data(), colorName.size()); // UTF-8
						}
					}
					break;
				}
				case Aseprite::ChunkType::UserDataChunk: {
					if (last != nullptr)
					{
						Aseprite::DWord flags = reader.read<Aseprite::DWord>();
						if ((flags & 0x01) == 0x01)
						{
							Aseprite::Word stringLength = reader.read<Aseprite::Word>();
							last->string.resize(stringLength);
							reader.read<char>(last->string.data(), last->string.size()); // UTF-8
						}
						if ((flags & 0x02) == 0x02)
						{
							last->color[0] = reader.read<Aseprite::Byte>();
							last->color[1] = reader.read<Aseprite::Byte>();
							last->color[2] = reader.read<Aseprite::Byte>();
							last->color[3] = reader.read<Aseprite::Byte>();
						}
					}
					else
					{
						reader.skim(chunkSize - chunkHeaderSize);
					}
					break;
				}
				case Aseprite::ChunkType::SliceChunk: {
					// Slice chunk
					Aseprite::DWord sliceKeyCount = reader.read<Aseprite::DWord>();
					Aseprite::DWord flags = reader.read<Aseprite::DWord>(); // 1 -> 9 patch slice, 2 -> has pivot information
					Aseprite::DWord reserved = reader.read<Aseprite::DWord>();
					std::string slicesName;
					{
						Aseprite::Word stringLength = reader.read<Aseprite::Word>();
						slicesName.resize(stringLength);
						reader.read<char>(slicesName.data(), slicesName.size()); // UTF-8
					}
					for (Aseprite::DWord iSliceKey = 0; iSliceKey < sliceKeyCount; iSliceKey++)
					{
						Aseprite::DWord frameNumber = reader.read<Aseprite::DWord>();
						Aseprite::Long xOrigin = reader.read<Aseprite::Long>();
						Aseprite::Long yOrigin = reader.read<Aseprite::Long>();
						Aseprite::DWord width = reader.read<Aseprite::DWord>(); // can be zero if hidden in the anim from the given frame)
						Aseprite::DWord height = reader.read<Aseprite::DWord>();
						if ((flags & 0x01) == 0x01)
						{
							Aseprite::Long centerX = reader.read<Aseprite::Long>();
							Aseprite::Long centerY = reader.read<Aseprite::Long>();
							Aseprite::DWord centerWidth = reader.read<Aseprite::DWord>();
							Aseprite::DWord centerHeight = reader.read<Aseprite::DWord>();
						}
						if ((flags & 0x02) == 0x02)
						{
							Aseprite::Long pivotXPosition = reader.read<Aseprite::Long>();
							Aseprite::Long pivotYPosition = reader.read<Aseprite::Long>();
						}
					}
					break;
				}
				default: {
					Logger::warn("Unsupported Aseprite chunk type. Skipping");
					reader.skim(chunkSize - chunkHeaderSize);
				}
				}
				ASSERT(reader.offset() - chunkOffset == chunkSize, "Invalid chunk reading");
			}
			ASSERT(reader.offset() - frameOffset == frameSize, "Invalid frame reading");
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

};