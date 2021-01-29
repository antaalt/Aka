#include "TextRenderSystem.h"

#include <Core/ECS/World.h>
#include <Core/Font.h>

#include "../Component/Text.h"
#include "../Component/Transform2D.h"

namespace aka {

TextRenderSystem::TextRenderSystem(World* world) :
    System(world)
{
}

void TextRenderSystem::draw(Batch& batch)
{
    const mat3f model = mat3f::identity();
    m_world->each<Text, Transform2D>([&](Entity* entity, Text* text, Transform2D* transform) {
        // iterate through all characters
        float scale = 1.f;
        float advance = transform->model[2].x + text->offset.x;
        for (const char& c : text->text)
        {
            // TODO check if rendering text out of screen for culling ?
            const Character& ch = text->font->character(c);
            vec2f position = vec2f(advance + ch.bearing.x, transform->model[2].y + text->offset.y - (ch.size.y - ch.bearing.y)) * scale;
            vec2f size = vec2f((float)ch.size.x, (float)ch.size.y) * scale;
            batch.draw(model, Batch::Rect(position, size, ch.texture.get(0), ch.texture.get(1), ch.texture.texture, text->color, text->layer));
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            advance += ch.advance * scale;
        }
    });
}

}