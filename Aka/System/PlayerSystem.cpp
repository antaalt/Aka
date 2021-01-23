#include "PlayerSystem.h"

#include "../Platform/Logger.h"
#include "../Core/ECS/World.h"
#include "../Component/Transform2D.h"
#include "../Component/Coin.h"
#include "../Component/Player.h"
#include "../Component/Animator.h"
#include "../Component/Text.h"
#include "SoundSystem.h"

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
			player->state = Player::State::Idle;
			event->resolve();
		}
	});
	m_world->each<Player, Transform2D, RigidBody2D, Animator>([&](Entity* entity, Player* player, Transform2D* transform, RigidBody2D* rigid, Animator* animator) {
		player->jump.update(deltaTime);
		player->left.update(deltaTime);
		player->right.update(deltaTime);

		player->ground = false;
		if (player->state == Player::State::Jumping || player->state == Player::State::DoubleJumping)
		{
			if (player->left.pressed())
			{
				animator->flipU = true;
				rigid->velocity.x = -player->speed.metric();
			}
			else if (player->right.pressed())
			{
				animator->flipU = false;
				rigid->velocity.x = player->speed.metric();
			}
			else if (player->right.up() || player->left.up())
			{
				rigid->velocity = vec2f(0.f);
			}

			if (player->jump.down() && player->state == Player::State::Jumping)
			{
				m_world->createEntity()->add<SoundInstance>(SoundInstance(Asset::path("sounds/jump.mp3"), 1.f));
				player->state = Player::State::DoubleJumping;
				rigid->acceleration.y = 0.f;
				rigid->velocity.y = 16.f;
			}
		}
		else
		{
			if (player->left.pressed())
			{
				animator->flipU = true;
				player->state = Player::State::Walking;
				rigid->velocity.x = -player->speed.metric();
			}
			else if (player->right.pressed())
			{
				animator->flipU = false;
				player->state = Player::State::Walking;
				rigid->velocity.x = player->speed.metric();
			}
			else if (player->right.up() || player->left.up())
			{
				player->state = Player::State::Idle;
				rigid->velocity = vec2f(0.f);
			}

			if (player->jump.down())
			{
				m_world->createEntity()->add<SoundInstance>(SoundInstance(Asset::path("sounds/jump.mp3"), 1.f));
				player->state = Player::State::Jumping;
				rigid->velocity.y = 16.f;
			}
		}
		/*switch (player->state)
		{
		case Player::State::Idle:
			Logger::info("Idle");
			break;
		case Player::State::Walking:
			Logger::info("Walking");
			break;
		case Player::State::Falling:
			Logger::info("Falling");
			break;
		case Player::State::Jumping:
			Logger::info("Jumping");
			break;
		}*/

		if (input::pressed(input::Key::LeftCtrl))
		{
			transform->model[2] = col3f(80, 224, 1);
		}
	});
}

};