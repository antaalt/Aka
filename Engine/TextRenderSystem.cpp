#include "TextRenderSystem.h"

#include "Text.h"
#include "Transform2D.h"
#include "World.h"

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
        float advance = transform->position.x;
        for (const char& c : text->text)
        {
            // TODO check if rendering text out of screen for culling ?
            const Character& ch = text->font->characters[c];
            vec2f position = vec2f(advance + ch.bearing.x, transform->position.y - (ch.size.y - ch.bearing.y)) * transform->size;
            vec2f size = vec2f((float)ch.size.x, (float)ch.size.y) * transform->size;
            batch.draw(model, Batch::Rect(position, size, ch.texture, text->layer));
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            advance += (ch.advance >> 6) * transform->size.x; // bitshift by 6 to get value in pixels (2^6 = 64)
        }
    });
}

}