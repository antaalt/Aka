#include <Aka/Resource/Resource/Texture.h>

#include <Aka/Core/Enum.h>
#include <Aka/OS/Logger.h>
#include <Aka/Resource/Archive/TextureArchive.h>

namespace aka {

gfx::TextureFormat getFormat(uint32_t channels, bool isHDR, bool isSRGB)
{
	AKA_ASSERT(isSRGB == false, "SRGB not supported yet.");
	if (isHDR)
	{
		gfx::TextureFormat formats[4]{
			gfx::TextureFormat::R32F,
			gfx::TextureFormat::RG32F,
			gfx::TextureFormat::RGB32F,
			gfx::TextureFormat::RGBA32F,
		};
		return formats[channels - 1];
	}
	else
	{
		gfx::TextureFormat formats[4]{
			gfx::TextureFormat::R8U,
			gfx::TextureFormat::RG8U,
			gfx::TextureFormat::RGB8U,
			gfx::TextureFormat::RGBA8U,
		};
		return formats[channels - 1];
	}
	// SRGB not supported for now
	return gfx::TextureFormat::Unknown;
}

Texture::Texture() :
	Resource(ResourceType::Texture)
{
}

Texture::~Texture()
{
}

void Texture::createRenderData(gfx::GraphicDevice* device, const BuildData* inBuildData)
{
	if (m_renderData != nullptr)
		return;
	if (inBuildData == nullptr)
		return;

	TextureRenderData* textureRenderData = new TextureRenderData;
	m_renderData = textureRenderData;

	// Texture type
	const TextureBuildData* data = reinterpret_cast<const TextureBuildData*>(inBuildData);
	const bool isCubemapTexture2D = (data->layers == 6) && has(data->flags, TextureBuildFlag::Cubemap);
	const bool isLayeredTexture2D = !isCubemapTexture2D && data->layers > 1;

	// Texture data
	const bool isHDR = has(data->flags, TextureBuildFlag::ColorSpaceHDR);
	const bool isSRGB = has(data->flags, TextureBuildFlag::ColorSpaceSRGB);
	const bool hasMips = has(data->flags, TextureBuildFlag::GenerateMips);
	gfx::TextureFormat format = getFormat(data->channels, isHDR, isSRGB);
	gfx::TextureFlag flags = gfx::TextureFlag::ShaderResource;
	uint32_t level = 1;
	if (hasMips)
	{
		flags |= gfx::TextureFlag::GenerateMips;
		level = gfx::Sampler::mipLevelCount(data->width, data->height);
	}

	if (isCubemapTexture2D)
	{
		if (data->layers != 6)
			return;
		const void* bytes[6] = {
			data->layerData(0),
			data->layerData(1),
			data->layerData(2),
			data->layerData(3),
			data->layerData(4),
			data->layerData(5)
		};
		textureRenderData->handle = device->createTexture(
			"FileTextureCubemap", // TODO custom name based on path ?
			data->width,
			data->height,
			1,
			gfx::TextureType::TextureCubeMap,
			level,
			6,
			format,
			flags,
			bytes
		);
	}
	else if (isLayeredTexture2D)
	{
		Vector<const void*> bytes(data->layers);
		for (uint32_t i = 0; i < data->layers; i++)
		{
			bytes[i] = data->layerData(i);
		}
		textureRenderData->handle = device->createTexture(
			"FileTexture2DArray",
			data->width,
			data->height,
			1,
			gfx::TextureType::TextureCubeMap,
			level,
			6,
			format,
			flags,
			bytes.data()
		);
	}
	else // isTexture2D
	{
		if (data->layers != 1)
			return;
		const void* bytes = data->data();
		textureRenderData->handle = device->createTexture(
			"FileTexture2D",
			data->width,
			data->height,
			1,
			gfx::TextureType::Texture2D,
			level,
			1,
			format,
			flags,
			&bytes
		);
	}
}

void Texture::destroyRenderData(gfx::GraphicDevice* device)
{
	if (m_renderData == nullptr)
		return;
	TextureRenderData* textureRenderData = reinterpret_cast<TextureRenderData*>(m_renderData);
	device->destroy(textureRenderData->handle);
	textureRenderData->handle = gfx::TextureHandle::null;
}

void Texture::createBuildData()
{
	if (m_buildData != nullptr)
		return;
	m_buildData = new TextureBuildData;
}

void Texture::createBuildData(gfx::GraphicDevice* device, RenderData* data)
{
	if (m_buildData != nullptr)
		return;
	TextureBuildData* textureBuildData = new TextureBuildData;
	m_buildData = textureBuildData;

	gfx::TextureHandle texture = reinterpret_cast<TextureRenderData*>(data)->handle;
	textureBuildData->channels = 4; // GPU textures only 4 channels.
	const bool isSRGB = false; // TODO check flags
	const bool isHDR = false; // TODO check format
	Logger::warn("SRGB & HDR not supported.");
	const bool generateMips = has(texture.data->flags, gfx::TextureFlag::GenerateMips);
	textureBuildData->flags = TextureBuildFlag::None;
	textureBuildData->width = texture.data->width;
	textureBuildData->height = texture.data->height;
	uint32_t componentSize = 1;
	if (generateMips)
	{
		textureBuildData->flags |= TextureBuildFlag::GenerateMips;
	}
	if (isHDR)
	{
		textureBuildData->flags |= TextureBuildFlag::ColorSpaceHDR;
		componentSize = 4;
	}
	if (isSRGB)
	{
		textureBuildData->flags |= TextureBuildFlag::ColorSpaceSRGB;
	}
	switch (texture.data->type)
	{
	case gfx::TextureType::Texture2D: {
		textureBuildData->layers = 1;
		textureBuildData->bytes.resize(textureBuildData->width * textureBuildData->height * textureBuildData->channels * componentSize * textureBuildData->layers);
		device->download(texture, textureBuildData->data(), 0, 0, texture.data->width, texture.data->height);
		break;
	}
	case gfx::TextureType::TextureCubeMap: {
		textureBuildData->layers = 6;
		textureBuildData->flags |= TextureBuildFlag::Cubemap;
		textureBuildData->bytes.resize(textureBuildData->width * textureBuildData->height * textureBuildData->channels * componentSize * textureBuildData->layers);
		for (uint32_t i = 0; i < 6; i++)
		{
			device->download(texture, textureBuildData->layerData(i), 0, 0, texture.data->width, texture.data->height, 0, i);
		}
		break;
	}
	case gfx::TextureType::Texture2DArray: {
		textureBuildData->layers = texture.data->layers;
		textureBuildData->bytes.resize(textureBuildData->width * textureBuildData->height * textureBuildData->channels * componentSize * textureBuildData->layers);
		for (uint32_t i = 0; i < textureBuildData->layers; i++)
		{
			device->download(texture, textureBuildData->layerData(i), 0, 0, texture.data->width, texture.data->height, 0, i);
		}
	}
	default:
		Logger::error("Texture type not supported");
		break;
	}
}

void Texture::destroyBuildData()
{
	if (m_buildData == nullptr)
		return;
	delete m_buildData;
	m_buildData = nullptr;
}

ResourceArchive* Texture::createResourceArchive()
{
	return new TextureArchive;
}

uint32_t Texture::width() const
{
	return reinterpret_cast<const TextureRenderData*>(m_renderData)->handle.data->width;
}
uint32_t Texture::height() const
{
	return reinterpret_cast<const TextureRenderData*>(m_renderData)->handle.data->height;
}
gfx::TextureType Texture::type() const
{
	return reinterpret_cast<const TextureRenderData*>(m_renderData)->handle.data->type;
}
gfx::TextureHandle Texture::handle() const
{
	if (m_renderData == nullptr)
		return gfx::TextureHandle::null;
	return reinterpret_cast<const TextureRenderData*>(m_renderData)->handle;
}

};