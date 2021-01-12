#pragma once

#include "Sprite.h"
#include "Component.h"

namespace app {

class SpriteAnimatorComponent : public Component
{
public:
	SpriteAnimatorComponent();
	~SpriteAnimatorComponent();

	// Set the sprite linked to the component
	void set(Sprite* sprite);
	// Play the given animation
	void play(const std::string& animation);
	// Get the current sprite frame animated
	Sprite::Frame &getCurrentSpriteFrame() const;

public:
	void create(GraphicBackend& backend) override;
	void destroy(GraphicBackend& backend) override;

	void update() override;
	void render(const Camera2D &camera, GraphicBackend& backend) override;

private:
	Sprite *m_sprite;
	uint32_t m_currentAnimation;
	uint32_t m_currentFrame;
	Time::unit m_animationStartTick;
	Time::unit m_currentAnimationDuration;

private:
	Shader m_shader;
	uint32_t m_vao;
};

};