#pragma once

#include "Sprite.h"
#include "Component.h"

namespace app {

class AnimatorSystem;

struct Animator : public Component
{
	friend AnimatorSystem;

	Animator();
	Animator(Sprite* sprite, float depth);

	Sprite* sprite;
	uint32_t currentAnimation;
	uint32_t currentFrame;
	float depth;

	Sprite::Frame& getCurrentSpriteFrame() const;

	void play(const std::string& animation);
private:
	Time::Unit animationStartTick;
	Time::Unit currentAnimationDuration;
};

};

