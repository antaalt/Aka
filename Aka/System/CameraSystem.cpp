#include "CameraSystem.h"

#include "../Core/ECS/World.h"
#include "../Component/Transform2D.h"
#include "../Platform/Input.h"

namespace aka {

CameraSystem::CameraSystem(World* world) :
	System(world)
{
}

void CameraSystem::update(Time::Unit deltaTime)
{
	m_world->each<Transform2D>([deltaTime](Entity* entity, Transform2D* transform){
		vec2f translation;
		translation.x = (input::pressed(input::Key::ArrowRight) - input::pressed(input::Key::ArrowLeft)) * 64.f * deltaTime.seconds();
		translation.y = (input::pressed(input::Key::ArrowUp) - input::pressed(input::Key::ArrowDown)) * 64.f * deltaTime.seconds();
		transform->move(translation);
	});
}

}