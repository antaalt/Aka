#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Resource.h>

#include <type_traits>

namespace aka {

struct SpriteRenderData : RenderData
{
	struct Frame {
		uint32_t layer;
		Time duration;
	};
	struct Animation {
		String name;
		Vector<Frame> frames;
	};

	Vector<Animation> animations;
	gfx::TextureHandle handle; // Texture array to hold all sprites

	uint32_t width;
	uint32_t height;

	// Get the layer of the texture depending on the anim.
	uint32_t getTextureLayer(const char* name, uint32_t frameIndex) const;
};

struct SpriteBuildData : BuildData
{
	uint32_t width;
	uint32_t height;
	struct Frame {
		Blob bytes; // rgba8
		Time duration;
	};
	struct Animation {
		String name;
		Vector<Frame> frames;
	};
	Vector<Animation> animations;

	Animation& find(const char* name);
};

struct SpriteFrame
{
	gfx::TextureHandle handle;
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
	~Sprite();

	void createBuildData() override;
	void createBuildData(gfx::GraphicDevice* device, RenderData* data) override;
	void createRenderData(gfx::GraphicDevice* device, const BuildData* data) override;
	void destroyRenderData(gfx::GraphicDevice* device) override;
	ResourceArchive* createResourceArchive() override;

	uint32_t getAnimationCount() const;
	SpriteAnimation& getAnimation(const char* name);
	SpriteAnimation& getAnimation(uint32_t index);
	const SpriteAnimation& getAnimation(const char* name) const;
	const SpriteAnimation& getAnimation(uint32_t index) const;
	uint32_t getAnimationIndex(const char* name) const;

	const SpriteFrame& getFrame(const char* name, uint32_t frame) const;
	const SpriteFrame& getFrame(uint32_t index, uint32_t frame) const;

	gfx::TextureHandle getTextureHandle() const;
	uint32_t getTextureLayer(const char* name, uint32_t frameIndex) const;

private:
	Vector<SpriteAnimation> m_animations;
};

};