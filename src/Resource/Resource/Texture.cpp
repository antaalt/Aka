#include <Aka/Resource/Resource/Texture.hpp>

#include <Aka/Renderer/Renderer.hpp>

namespace aka {

Texture::Texture() : 
	Resource(ResourceType::Texture),
	m_textureID(TextureID::Invalid),
	m_width(0), 
	m_height(0),
	m_textureHandle(gfx::TextureHandle::null),
	m_textureType(gfx::TextureType::Unknown),
	m_textureUsage(gfx::TextureUsage::Unknown),
	m_textureFormat(gfx::TextureFormat::Unknown),
	m_layerCount(0),
	m_mipCount(0)
{
}

Texture::Texture(AssetID _id, const String& _name) :
	Resource(ResourceType::Texture, _id, _name),
	m_textureID(TextureID::Invalid),
	m_width(0),
	m_height(0),
	m_textureHandle(gfx::TextureHandle::null),
	m_textureType(gfx::TextureType::Unknown),
	m_textureUsage(gfx::TextureUsage::Unknown),
	m_textureFormat(gfx::TextureFormat::Unknown),
	m_layerCount(0),
	m_mipCount(0)
{
}

Texture::~Texture()
{
	// TODO release texture
}

gfx::TextureFormat getTextureFormat(ArchiveImageFormat format)
{
	switch (format)
	{
	default:
		AKA_NOT_IMPLEMENTED;
	case ArchiveImageFormat::Uncompressed:
	case ArchiveImageFormat::Compressed:
		return gfx::TextureFormat::RGBA8;
	case ArchiveImageFormat::Bc1: return gfx::TextureFormat::Bc1;
	case ArchiveImageFormat::Bc2: return gfx::TextureFormat::Bc2;
	case ArchiveImageFormat::Bc3: return gfx::TextureFormat::Bc3;
	case ArchiveImageFormat::Bc4: return gfx::TextureFormat::Bc4;
	case ArchiveImageFormat::Bc5: return gfx::TextureFormat::Bc5;
	}
}

void Texture::fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer)
{
	const ArchiveImage& imageArchive = _context.getArchive<ArchiveImage>(getID());
	// TODO custom mips
	const void* data = imageArchive.data.data();
	m_width = imageArchive.width;
	m_height = imageArchive.height;
	m_textureFormat = aka::getTextureFormat(imageArchive.format);
	m_textureType = gfx::TextureType::Texture2D;
	m_textureUsage = gfx::TextureUsage::ShaderResource;
	// Compressed texture should have their own mips (generation doesnt work for now).
	if (!gfx::Texture::isCompressed(m_textureFormat))
		m_textureUsage |= gfx::TextureUsage::GenerateMips;
	m_layerCount = 1; // TODO
	m_mipCount = gfx::Sampler::mipLevelCount(imageArchive.width, imageArchive.height);
	m_textureHandle = _renderer->getDevice()->createTexture(
		"AlbedoTexture", 
		imageArchive.width, imageArchive.height, 1, 
		m_textureType,
		m_mipCount, m_layerCount,
		m_textureFormat,
		m_textureUsage,
		&data
	);
	m_textureID = _renderer->allocateTextureID(m_textureHandle);
}

void Texture::toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer)
{
	AKA_NOT_IMPLEMENTED;
}

void Texture::destroy_internal(AssetLibrary* _library, Renderer* _renderer)
{
	_renderer->getDevice()->destroy(m_textureHandle);
}

}