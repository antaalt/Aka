#include "AnimatorSystem.h"

namespace app {

void AnimatorSystem::update()
{
    for (Entity* entity : m_entities)
    {
        Animator* animator = entity->get<Animator>();
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
    }
}

bool AnimatorSystem::valid(Entity* entity)
{
	return entity->has<Animator>();
}

}
