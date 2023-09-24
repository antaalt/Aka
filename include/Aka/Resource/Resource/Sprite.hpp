#pragma once 

#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Core/Container/String.h>
#include <Aka/Graphic/GraphicDevice.h>

namespace aka {

struct ArchiveSprite;
class AssetLibrary;
class Renderer;


struct SpriteFrame
{
	uint32_t layer;
	uint32_t width;
	uint32_t height;
	Time duration; // Duration of the frame
};

struct SpriteAnimation
{
	String name;
	Vector<SpriteFrame> frames;

	SpriteFrame& operator[](size_t index);
	const SpriteFrame& operator[](size_t index) const;

	Time getDuration() const; // Duration of the animation
};

class Sprite : public Resource
{
public:
	Sprite();
	Sprite(ResourceID _id, const String& _name);
	~Sprite();
private:
	void create_internal(AssetLibrary* library, Renderer* _renderer, const Archive& _archive) override;
	void save_internal(AssetLibrary* library, Renderer* _renderer, Archive& _archive) override;
	void destroy_internal(AssetLibrary* library, Renderer* _renderer) override;
public:
	uint32_t getAnimationCount() const;
	SpriteAnimation& getAnimation(uint32_t i);
	const SpriteAnimation& getAnimation(uint32_t i) const;
	SpriteAnimation& getAnimation(const char* name);
	const SpriteAnimation& getAnimation(const char* name) const;
	uint32_t getAnimationIndex(const char* name) const;
	gfx::TextureHandle getTextureHandle() const;
	uint32_t getTextureLayer(const char* name, uint32_t frameIndex) const;
	const SpriteFrame& getFrame(uint32_t index, uint32_t frame) const;
	const SpriteFrame& getFrame(const char* name, uint32_t frame) const;
private:
	gfx::TextureHandle m_handle;
	Vector<SpriteAnimation> m_animations;
};

};