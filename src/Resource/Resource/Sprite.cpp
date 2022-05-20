#include <Aka/Resource/Resource/Sprite.h>

#include <Aka/Core/Enum.h>
#include <Aka/OS/Logger.h>
#include <Aka/Resource/Archive/SpriteArchive.h>

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

SpriteBuildData::Animation& SpriteBuildData::find(const char* name)
{
	for (auto& animation : animations)
	{
		if (animation.name == String(name))
		{
			return animation;
		}
	}
	return animations[0]; // default if not found
}

Sprite::Sprite() :
	Resource(ResourceType::Sprite)
{

}

Sprite::~Sprite()
{
}

void Sprite::createRenderData(gfx::GraphicDevice* device, const BuildData* inBuildData)
{
	if (m_renderData != nullptr)
		return;
	if (inBuildData == nullptr)
		return;

	SpriteRenderData* spriteRenderData = new SpriteRenderData;
	m_renderData = spriteRenderData;

	// Texture type
	const SpriteBuildData* data = reinterpret_cast<const SpriteBuildData*>(inBuildData);
	Vector<const void*> bytes;
	uint32_t width = data->width;
	uint32_t height = data->height;
	uint32_t layerCount = 0;
	for (const SpriteBuildData::Animation& animation : data->animations)
	{
		SpriteRenderData::Animation renderAnimation;
		renderAnimation.name = animation.name;
		for (const SpriteBuildData::Frame& frame : animation.frames)
		{
			SpriteRenderData::Frame renderFrame{};
			renderFrame.duration = frame.duration;
			renderFrame.layer = layerCount++;
			renderAnimation.frames.append(renderFrame);
			bytes.append(frame.bytes.data());
		}
		spriteRenderData->animations.append(renderAnimation);
	}

	spriteRenderData->handle = device->createTexture(
		width,
		height,
		1,
		gfx::TextureType::Texture2DArray,
		1, // TODO mips ?
		layerCount,
		gfx::TextureFormat::RGBA8, // TODO format ?
		gfx::TextureFlag::ShaderResource,
		bytes.data()
	);

	// Fill data
	uint32_t layer = 0;
	for (const SpriteBuildData::Animation& buildAnimation : data->animations)
	{
		SpriteAnimation animation;
		animation.name = buildAnimation.name;
		for (const SpriteBuildData::Frame& buildFrame : buildAnimation.frames)
		{
			SpriteFrame frame{};
			frame.duration = buildFrame.duration;
			frame.handle = spriteRenderData->handle;
			frame.layer = layer++;
			frame.width = data->width;
			frame.height = data->height;
			
			animation.frames.append(frame);
		}
		m_animations.append(animation);
	}
}

void Sprite::destroyRenderData(gfx::GraphicDevice* device)
{
	if (m_renderData == nullptr)
		return;
	SpriteRenderData* textureRenderData = reinterpret_cast<SpriteRenderData*>(m_renderData);
	device->destroy(textureRenderData->handle);
	textureRenderData->handle = gfx::TextureHandle::null;
}

ResourceArchive* Sprite::createResourceArchive()
{
	return new SpriteArchive;
}

void Sprite::createBuildData()
{
	if (m_buildData != nullptr)
		return;
	m_buildData = new SpriteBuildData;
}

void Sprite::createBuildData(gfx::GraphicDevice* device, RenderData* data)
{
	if (m_buildData != nullptr)
		return;
	SpriteBuildData* spriteBuildData = new SpriteBuildData;
	m_buildData = spriteBuildData;
	// TODO generate data.
	AKA_NOT_IMPLEMENTED;
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
	return reinterpret_cast<const SpriteRenderData*>(getRenderData())->handle;
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