#include "TileRenderSystem.h"

#include "../Platform/IO/FileSystem.h"
#include "../Component/Camera2D.h"
#include "../Component/Transform2D.h"
#include "../Component/Animator.h"
#include "../Core/World.h"

namespace aka {

TileSystem::TileSystem(World* world) :
    System(world)
{
}

void TileSystem::draw(Batch &batch)
{
    const mat3f model = mat3f::identity();
    m_world->each<Animator, Transform2D>([&](Entity *entity, Animator *animator, Transform2D* transform) {
        uv2f uv0 = uv2f(0.f);
        uv2f uv1 = uv2f(1.f);
        if (animator->flipU)
        {
            uv0.u = 1.f - uv0.u;
            uv1.u = 1.f - uv1.u;
        }
        if (animator->flipV)
        {
            uv0.v = 1.f - uv0.v;
            uv1.v = 1.f - uv1.v;
        }
        batch.draw(model, Batch::Rect(transform->position, transform->size, uv0, uv1, animator->getCurrentSpriteFrame().texture, animator->layer));
    });
}

}
