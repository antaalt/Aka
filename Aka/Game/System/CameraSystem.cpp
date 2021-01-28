#include "CameraSystem.h"

#include "../../Core/ECS/World.h"
#include "../Component/Camera2D.h"
#include "../Component/Player.h"
#include "../Component/Transform2D.h"
#include "../Component/TileLayer.h"
#include "../../Platform/Input.h"
#include "../../OS/Logger.h"


namespace aka {

CameraSystem::CameraSystem(World* world) :
	System(world)
{
}

void CameraSystem::update(Time::Unit deltaTime)
{
	// Shake screen when hit receive
	/*m_world->receive<CollisionEvent>([](CollisionEvent* collision) {

	});*/
	m_world->each<Camera2D>([&](Entity* entity, Camera2D* camera) {
		// Track the player
		Player *player = m_world->first<Player>();
		const Transform2D* transform = player->getEntity()->get<Transform2D>();

		const float hThreshold = 0.4f * camera->viewport.x;
		const float vThreshold = 0.2f * camera->viewport.y;
		const vec2f playerPosition = transform->position();
		const vec2f playerRelativePosition = playerPosition - camera->position;
		// Horizontal
		if (playerRelativePosition.x < hThreshold)
		{
			float distance = abs<float>(playerRelativePosition.x - hThreshold);
			if (distance > 1.f)
				camera->position.x -= pow<float>(distance * 10.f * deltaTime.seconds(), 2.f);
		}
		else if (playerRelativePosition.x > camera->viewport.x - hThreshold)
		{
 			float distance = playerRelativePosition.x - (camera->viewport.x - hThreshold);
			if (distance > 1.f)
				camera->position.x += pow<float>(distance * 10.f * deltaTime.seconds(), 2.f);
		}

		// Vertical
		if (playerRelativePosition.y < vThreshold)
		{
			float distance = abs<float>(playerRelativePosition.y - vThreshold);
			if (distance > 1.f)
				camera->position.y -= pow<float>(distance * 10.f * deltaTime.seconds(), 2.f);
		}
		else if (playerRelativePosition.y > camera->viewport.y - vThreshold)
		{
			float distance = playerRelativePosition.y - (camera->viewport.y - vThreshold);
			if (distance > 1.f)
				camera->position.y += pow<float>(distance * 10.f * deltaTime.seconds(), 2.f);
		}

		// Clamp camera position to the current level bounds.
		TileLayer * layer = m_world->first<TileLayer>();
		vec2f grid = vec2f(layer->gridSize * layer->gridCount) + layer->offset;
		
		camera->position.x = max<float>(camera->position.x, layer->offset.x);
		camera->position.x = min<float>(camera->position.x, grid.x - camera->viewport.x);
		camera->position.y = max<float>(camera->position.y, layer->offset.y);
		camera->position.y = min<float>(camera->position.y, grid.y - camera->viewport.y);
	});
}

}