#include "CameraSystem.h"

#include "../Core/World.h"
#include "../Component/Transform2D.h"
#include "../Platform/Input.h"

namespace aka {

CameraSystem::CameraSystem(World* world) :
	System(world)
{
}

void CameraSystem::update(Time::Unit deltaTime)
{
	m_world->each<Transform2D>([](Entity* entity, Transform2D* transform){
		transform->position.x += input::pressed(input::Key::ArrowRight) - input::pressed(input::Key::ArrowLeft);
		transform->position.y += input::pressed(input::Key::ArrowUp) - input::pressed(input::Key::ArrowDown);
	});
}

}