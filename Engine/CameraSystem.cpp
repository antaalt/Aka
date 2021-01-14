#include "CameraSystem.h"

#include "World.h"
#include "Transform2D.h"
#include "Input.h"

namespace app {

CameraSystem::CameraSystem(World* world) :
	System(world)
{
}

void CameraSystem::update()
{
	m_world->each<Transform2D>([](Entity* entity, Transform2D* transform){
		transform->position.x += input::pressed(input::Key::ArrowRight) - input::pressed(input::Key::ArrowLeft);
		transform->position.y += input::pressed(input::Key::ArrowUp) - input::pressed(input::Key::ArrowDown);
	});
}

}