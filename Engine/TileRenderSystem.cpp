#include "TileRenderSystem.h"

#include "FileSystem.h"
#include "Camera2D.h"
#include "Transform2D.h"
#include "Animator.h"
#include "World.h"

namespace aka {

TileSystem::TileSystem(World* world) :
    System(world)
{
}

void TileSystem::render(GraphicBackend& backend, Batch &batch)
{
    // TODO push blend mode to batch
    m_world->each<Transform2D, Animator>([&](Entity *entity, Transform2D *transform, Animator* animator) {
        batch.pushLayer(animator->layer);
        batch.texture(mat3f::identity(), transform->position, transform->size, animator->getCurrentSpriteFrame().texture);
    });
}

}
