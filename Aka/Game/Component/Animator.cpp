#include "Animator.h"

#include "../../Core/Debug.h"

namespace aka {

Animator::Animator() :
    Animator(nullptr, 0)
{
}

Animator::Animator(Sprite* sprite, int32_t layer) :
    sprite(sprite),
    currentAnimation(0),
    currentFrame(0),
    layer(layer),
    flipU(false),
    flipV(false),
    animationTimer(),
    currentAnimationDuration()
{
}

Sprite::Frame& Animator::getCurrentSpriteFrame() const
{
    return sprite->animations[currentAnimation].frames[currentFrame];
}

void Animator::play(const std::string& animation)
{
    Sprite::Animation* a = sprite->getAnimation(animation);
    ASSERT(a != nullptr, "No valid animation");
    currentAnimation = static_cast<uint32_t>(a - sprite->animations.data());
    currentFrame = 0;
    animationTimer = Time::Unit();
    currentAnimationDuration = a->duration();
}

}