#include "AnimatorSystem.h"

#include "../Core/World.h"

namespace aka {

AnimatorSystem::AnimatorSystem(World* world) :
    System(world)
{
}

void AnimatorSystem::update(Time::Unit deltaTime)
{
    m_world->each<Animator>([deltaTime](Entity* entity, Animator* animator) {
        Time::Unit zero = Time::Unit();
        if (animator->currentAnimationDuration > zero && animator->sprite != nullptr)
        {
            animator->animationTimer = (animator->animationTimer + deltaTime) % animator->currentAnimationDuration;
            uint32_t frameID = 0;
            Time::Unit currentFrameDuration = zero;
            for (Sprite::Frame& frame : animator->sprite->animations[animator->currentAnimation].frames)
            {
                if (animator->animationTimer < currentFrameDuration + frame.duration)
                    break;
                currentFrameDuration += frame.duration;
                frameID++;
            }
            animator->currentFrame = frameID;
        }
    });
}

}