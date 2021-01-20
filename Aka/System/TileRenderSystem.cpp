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
    m_world->each<Animator>([&](Entity *entity, Animator *animator) {
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
        Transform2D* transform = entity->get<Transform2D>();
        Texture::Ptr texture = animator->getCurrentSpriteFrame().texture;
        vec2f position = vec2f(0.f);
        vec2f size = vec2f(texture->width(), texture->height());
        if (transform != nullptr)
        {
            position = transform->position;
            size = transform->size;
        }
        batch.draw(model, Batch::Rect(position, size, uv0, uv1, texture, animator->layer));
    });
}

}
