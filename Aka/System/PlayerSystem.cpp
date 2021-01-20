#include "PlayerSystem.h"

#include "../Platform/Logger.h"
#include "../Core/World.h"
#include "../Component/Transform2D.h"
#include "../Component/Interact.h"
#include "../Component/Player.h"
#include "../Component/Animator.h"
#include "../Component/Text.h"

namespace aka {

PlayerSystem::PlayerSystem(World* world) :
	System(world)
{
}
void PlayerSystem::update(Time::Unit deltaTime)
{
	// Inputs for player, find the player component ?
	m_world->receive<CollisionEvent>([](CollisionEvent* event) {
		Player* player = event->left->get<Player>();
		if (player == nullptr)
			return;
		Text* text = event->left->get<Text>();
		Coin* coin = event->right->get<Coin>();
		if (coin != nullptr)
		{
			if (!coin->picked)
			{
				player->coin++;
				coin->picked = true;
				event->right->get<Animator>()->play("picked");
				text->text = std::to_string(player->coin);
			}
		}
		else
		{
			event->resolve();
		}
	});
}

};