#include <Aka/Resource/Resource/Sprite.hpp>

#include <Aka/Core/Enum.h>
#include <Aka/OS/Logger.h>
#include <Aka/Resource/Archive/ArchiveSprite.hpp>
#include <Aka/Resource/Archive/ArchiveImage.hpp>
#include <Aka/Renderer/Renderer.hpp>

namespace aka {

SpriteFrame& aka::SpriteAnimation::operator[](size_t index)
{
	return frames[index];
}
const SpriteFrame& aka::SpriteAnimation::operator[](size_t index) const
{
	return frames[index];
}

Time SpriteAnimation::getDuration() const
{
	Time duration = Time::zero();
	for (auto& frame : frames)
		duration += frame.duration;
	return duration;
}

Sprite::Sprite() :
	Resource(ResourceType::Sprite)
{
}

Sprite::Sprite(AssetID _id, const String& _name) :
	Resource(ResourceType::Sprite, _id, _name)
{
}

Sprite::~Sprite()
{
}

void Sprite::fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer)
{
	const ArchiveSprite& archiveSprite = _context.getArchive<ArchiveSprite>(getID());
	
	const uint32_t width = archiveSprite.width;
	const uint32_t height = archiveSprite.height;
	Vector<const void*> bytes;
	uint32_t layerCount = 0;

	for (const ArchiveSpriteAnimation& archiveAnimation : archiveSprite.animations)
	{
		SpriteAnimation animation{};
		animation.name = archiveAnimation.name;
		for (const ArchiveSpriteFrame& archiveFrame : archiveAnimation.frames)
		{
			SpriteFrame frame{};
			frame.duration = archiveFrame.duration;
			frame.layer = layerCount++;
			animation.frames.append(frame);
			ArchiveImage& archiveImage = _context.getArchive<ArchiveImage>(archiveFrame.image);
			bytes.append(archiveImage.data.data());
		}
		m_animations.append(animation);
	}
	m_handle = _renderer->getDevice()->createTexture(
		"SpriteTexture", // TODO name
		width,
		height,
		1,
		gfx::TextureType::Texture2DArray,
		1,
		layerCount,
		gfx::TextureFormat::RGBA8,
		gfx::TextureUsage::ShaderResource,
		bytes.data()
	);
}
void Sprite::toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer)
{
	AKA_NOT_IMPLEMENTED;
}
void Sprite::destroy_internal(AssetLibrary* library, Renderer* _renderer)
{
	_renderer->getDevice()->destroy(m_handle);
	m_animations.clear();
}

uint32_t Sprite::getAnimationCount() const
{
	return static_cast<uint32_t>(m_animations.size());
}

SpriteAnimation& Sprite::getAnimation(const char* name)
{
	for (auto& animation : m_animations)
	{
		if (animation.name == String(name))
		{
			return animation;
		}
	}
	return m_animations[0]; // return first animation if not found
}

SpriteAnimation& Sprite::getAnimation(uint32_t index)
{
	return m_animations[index];
}

const SpriteAnimation& Sprite::getAnimation(const char* name) const
{
	for (auto& animation : m_animations)
	{
		if (animation.name == String(name))
		{
			return animation;
		}
	}
	return m_animations[0]; // return first animation if not found
}
const SpriteAnimation& Sprite::getAnimation(uint32_t index) const
{
	return m_animations[index];
}
uint32_t Sprite::getAnimationIndex(const char* name) const
{
	for (uint32_t i = 0; i < (uint32_t)m_animations.size(); i++)
	{
		auto animation = m_animations[i];
		if (animation.name == String(name))
		{
			return i;
		}
	}
	return ~0;
}
const SpriteFrame& Sprite::getFrame(const char* name, uint32_t frame) const
{
	return getAnimation(name)[frame];
}
const SpriteFrame& Sprite::getFrame(uint32_t index, uint32_t frame) const
{
	return m_animations[index][frame];
}
gfx::TextureHandle Sprite::getTextureHandle() const
{
	return m_handle;
}
uint32_t Sprite::getTextureLayer(const char* name, uint32_t frameIndex) const
{
	uint32_t layer = 0;
	for (auto& animation : m_animations)
	{
		if (animation.name == String(name))
		{
			AKA_ASSERT(frameIndex < animation.frames.size(), "Frame out of bounds");
			return layer + frameIndex;
		}
		layer += (uint32_t)animation.frames.size();
	}
	return ~0; // Animation not found
}

};