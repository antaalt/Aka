#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Resource.h>

#include <type_traits>

namespace aka {

enum class TextureBuildFlag : uint8_t
{
	None = 0,
	ColorSpaceSRGB = 1 << 0,
	ColorSpaceHDR = 1 << 1,
	GenerateMips = 1 << 2,
	Cubemap = 1 << 3,
	Compressed = 1 << 4, // TODO KTX & DDS
};

AKA_IMPLEMENT_BITMASK_OPERATOR(TextureBuildFlag);

struct TextureRenderData : RenderData
{
	gfx::TextureHandle handle;
};

struct TextureBuildData : BuildData
{
	Vector<byte_t> bytes; // Data of textures 
	uint32_t layers; // Number of layers
	uint32_t width, height; // Dimension
	uint8_t channels; // Channels
	TextureBuildFlag flags; // Build flags

	// Size of the memory
	size_t size() const { return width * height * channels; }
	// Data of the whole image
	byte_t* data() { return bytes.data(); }
	const byte_t* data() const { return bytes.data(); }
	// Data of a single layer
	byte_t* layerData(uint32_t layer) { return data() + size() * layer; }
	const byte_t* layerData(uint32_t layer) const { return data() + size() * layer; }
};

class Texture : public Resource
{
public:
	Texture();
	~Texture();

	void createBuildData() override;
	void createBuildData(gfx::GraphicDevice* device, RenderData* data) override;
	void destroyBuildData() override;
	void createRenderData(gfx::GraphicDevice* device, const BuildData* data) override;
	void destroyRenderData(gfx::GraphicDevice* device) override;
	ResourceArchive* createResourceArchive() override;

	uint32_t width() const;
	uint32_t height() const;
	gfx::TextureType type() const;
	gfx::TextureHandle handle() const;
private:
	uint32_t m_width;
	uint32_t m_height;
	gfx::TextureType m_textureType;
};

};