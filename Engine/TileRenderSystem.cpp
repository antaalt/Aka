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
    const mat3f model = mat3f::identity();
    m_world->each<Animator, Transform2D>([&](Entity *entity, Animator *animator, Transform2D* transform) {
        batch.draw(model, Batch::Rect(transform->position, transform->size, animator->getCurrentSpriteFrame().texture, animator->layer));
    });
}

}
