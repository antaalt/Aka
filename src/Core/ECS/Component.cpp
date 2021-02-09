#include <Aka/Core/ECS/Component.h>

namespace aka {

uint8_t Component::Type::m_typeCounter = 0;

uint8_t Component::getType() const
{
    return m_type;
}

const Entity* Component::getEntity() const
{
    return m_entity;
}

};