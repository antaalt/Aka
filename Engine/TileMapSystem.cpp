#include "TileMapSystem.h"

#include "FileSystem.h"
#include "Camera2D.h"
#include "Transform2D.h"
#include "Animator.h"
#include "World.h"

namespace aka {

TileMapSystem::TileMapSystem(World* world) :
    System(world)
{
}

void TileMapSystem::render(GraphicBackend &backend, Batch& batch)
{
    m_world->each<Transform2D, TileMap, TileLayer>([&](Entity* entity, Transform2D *transform, TileMap* atlas, TileLayer* layer)
    {
        // TODO alpha
        ASSERT(layer->gridSize == atlas->gridSize, "");
        for (size_t i = 0; i < layer->tileID.size(); i++)
        {
            // Ogmo tileID is top left to bottom while opengl pos is bottom to top.
            vec2u tileID = vec2u(i % layer->gridCount.x, layer->gridCount.y - 1 -  i / layer->gridCount.x);
            vec2f position = transform->position + vec2f(tileID * layer->gridSize);
            vec2f size = vec2f(layer->gridSize);
            // Get the uv
            int32_t atlasIDUnique = layer->tileID[i];
            if (atlasIDUnique == -1)
                continue;
            vec2u atlasID = vec2u(atlasIDUnique % atlas->gridCount.x, atlasIDUnique / atlas->gridCount.x);
            // uv flip function
            uv2f start = uv2f(vec2f(atlasID) / vec2f(atlas->gridCount));
            uv2f end = start + uv2f(vec2f(1.f) / vec2f(atlas->gridCount));
            batch.texture(mat3f::identity(), position, size, uv2f(start.u, 1.f - end.v), uv2f(end.u, 1.f - start.v) , atlas->texture);
        }
        batch.rect(mat3f::identity(), transform->position, transform->position + vec2f(atlas->gridSize * atlas->gridCount), color4f(1.f));
    });
}

}
