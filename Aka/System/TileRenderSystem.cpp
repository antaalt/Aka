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
        batch.draw(model, Batch::Rect(transform->position, transform->size, animator->getCurrentSpriteFrame().texture, animator->layer));
    });
}

}
