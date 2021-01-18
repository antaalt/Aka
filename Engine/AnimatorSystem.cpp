#include "AnimatorSystem.h"

#include "World.h"

namespace aka {

AnimatorSystem::AnimatorSystem(World* world) :
    System(world)
{
}

void AnimatorSystem::update(Time::Unit deltaTime)
{
    m_world->each<Animator>([](Entity* entity, Animator* animator) {
        if (animator->currentAnimationDuration > 0)
        {
            const Time::Unit now = Time::now();
            Time::Unit elapsedSincePlay = (now - animator->animationStartTick) % animator->currentAnimationDuration;
            Time::Unit currentFrameDuration = 0;
            uint32_t frameID = 0;
            while (elapsedSincePlay > (currentFrameDuration = animator->sprite->animations[animator->currentAnimation].frames[frameID].duration))
            {
                elapsedSincePlay -= currentFrameDuration;
                frameID = (frameID + 1) % animator->sprite->animations[animator->currentAnimation].frames.size();
            }
            animator->currentFrame = frameID;
        }
    });
}

}
