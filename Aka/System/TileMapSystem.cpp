#include "TileMapSystem.h"

#include "../Platform/IO/FileSystem.h"
#include "../Component/Camera2D.h"
#include "../Component/Transform2D.h"
#include "../Component/Animator.h"
#include "../Component/TileMap.h"
#include "../Component/Tilelayer.h"
#include "../Core/World.h"

namespace aka {

TileMapSystem::TileMapSystem(World* world) :
    System(world)
{
}

void TileMapSystem::draw(Batch& batch)
{
    m_world->each<Transform2D, TileMap, TileLayer>([&](Entity* entity, Transform2D *transform, TileMap* atlas, TileLayer* layer)
    {
        if (atlas->texture == nullptr)
            return;
        ASSERT(layer->gridSize == atlas->gridSize, "");
        for (size_t i = 0; i < layer->tileID.size(); i++)
        {
            // Ogmo tileID is top left to bottom while opengl projection is bottom to top.
            vec2u tileID = vec2u((uint32_t)i % layer->gridCount.x, layer->gridCount.y - 1 - (uint32_t)i / layer->gridCount.x);
            vec2f position = layer->offset + vec2f(tileID * layer->gridSize);
            vec2f size = vec2f(layer->gridSize);
            int32_t atlasIDUnique = layer->tileID[i];
            if (atlasIDUnique == -1)
                continue;
            vec2u atlasID = vec2u(atlasIDUnique % atlas->gridCount.x, atlasIDUnique / atlas->gridCount.x);
            uv2f start = uv2f(vec2f(atlasID) / vec2f(atlas->gridCount));
            uv2f end = start + uv2f(vec2f(1.f) / vec2f(atlas->gridCount));
            batch.draw(transform->model, Batch::Rect(position, size, start, end, atlas->texture, layer->layer));
        }
    });
}

}
