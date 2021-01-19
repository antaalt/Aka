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

void PlayerSystem::create()
{
	m_world->subscribe<CollisionEvent>(this);
}

void PlayerSystem::destroy()
{
	m_world->unsubscribe<CollisionEvent>(this);
}

void PlayerSystem::update(Time::Unit deltaTime)
{
	// Inputs for player, find the player component ?
}

void PlayerSystem::receive(World* world, const CollisionEvent& collision)
{
	Player* player = collision.left->get<Player>();
	if (player == nullptr)
		return;
	Text* text = collision.left->get<Text>();
	Coin* coin = collision.right->get<Coin>();
	if (coin != nullptr)
	{
		if (!coin->picked)
		{
			player->coin++;
			coin->picked = true;
			collision.right->get<Animator>()->play("picked");
			text->text = std::to_string(player->coin);
		}
	}
	else
	{
		collision.resolve();
	}
}

};