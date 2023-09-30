#include <Aka/Resource/Resource/Texture.hpp>

#include <Aka/Renderer/Renderer.hpp>

namespace aka {

Texture::Texture() : 
	Resource(ResourceType::Texture),
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

void Texture::fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer)
{
	const ArchiveImage& imageArchive = _context.getArchive<ArchiveImage>(getID());	
	// TODO custom mips
	const void* data = imageArchive.data.data();
	m_width = imageArchive.width;
	m_height = imageArchive.height;
	m_textureType = gfx::TextureType::Texture2D;
	m_textureFormat = gfx::TextureFormat::RGBA8;
	m_textureUsage = gfx::TextureUsage::ShaderResource | gfx::TextureUsage::GenerateMips;
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