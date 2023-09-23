#include <Aka/Scene/Component.hpp>

namespace aka {

uint32_t Component::s_globalComponentID = 0;

Component::Component(ComponentID id) :
	m_id(id)
{

}

};