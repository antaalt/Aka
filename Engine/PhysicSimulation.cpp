#include "PhysicSimulation.h"

#include "Image.h"

namespace app {

SpriteAnimatorComponent debugSpriteAnimator;
Sprite debugSprite;

PhysicSimulation::PhysicSimulation(float timestep) :
	m_timestep(timestep),
	m_lastTick(Time::now()),
	m_running(false)
{
}

void PhysicSimulation::create(GraphicBackend& backend)
{
	Image image = Image::load(Asset::path("textures/debug/collider.png"));
	Sprite::Animation anim;
	anim.name = "idle";
	anim.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 0));
	debugSprite.animations.push_back(anim);
	debugSprite.position = vec2f(0, 0);
	debugSprite.size = vec2f(16, 16);
	debugSprite.rotation = degreef(0);
	debugSpriteAnimator.set(&debugSprite);
	debugSpriteAnimator.create(backend);
	debugSpriteAnimator.play("idle");
}

void PhysicSimulation::destroy(GraphicBackend& backend)
{
}

void PhysicSimulation::start()
{
	m_running = true;
	m_lastTick = Time::now();
}

void PhysicSimulation::pause()
{
	m_running = false;
}

void PhysicSimulation::update()
{
	if (!m_running)
		return;

	const Time::unit now = Time::now();
	const vec2f force = vec2f(0.f, -9.81f);
	float dt = m_timestep;
	// dt is the timestep
	while (m_lastTick + dt < now)
	{
		// Update velocity vectors
		for (DynamicCollider2D* dynamic : m_dynamics)
		{
			dynamic->addForce(force, dt);
		}
		// Move dynamic objects
		for (DynamicCollider2D* dynamic : m_dynamics)
		{
			// Move rigid
			dynamic->move(dt);
			// Check collisions with static objects
			for (StaticCollider2D* collider : m_statics)
			{
				Collision c = dynamic->overlaps(*collider);
				if (c.collided)
				{
					dynamic->resolve(c.separation);
				}
			}
			// Check collisions with other dynamic object
			for (DynamicCollider2D* otherDynamic : m_dynamics)
			{
				// Skip self intersection
				if (otherDynamic == dynamic)
					continue;
				Collision c = dynamic->overlaps(*otherDynamic);
				if (c.collided)
				{
					dynamic->resolve(c.separation);
					// TODO resolve other dynamic
				}
			}
		}
		m_lastTick += static_cast<Time::unit>(dt * 1000.f);
	}
}
void PhysicSimulation::render(const Camera2D &camera, GraphicBackend& backend)
{
	// For debug
	for (DynamicCollider2D* dynamic : m_dynamics)
	{
		debugSprite.position = dynamic->getShape()->getPosition();
		debugSprite.size = dynamic->getShape()->getSize();
		debugSpriteAnimator.render(camera, backend);
	}
	for (StaticCollider2D* statics : m_statics)
	{
		debugSprite.position = statics->getShape()->getPosition();
		debugSprite.size = statics->getShape()->getSize();
		debugSpriteAnimator.render(camera, backend);
	}
}
DynamicRectCollider2D* PhysicSimulation::createDynamicRectCollider()
{
	DynamicRectCollider2D *rect = new DynamicRectCollider2D;
	m_dynamics.push_back(rect);
	m_colliders.push_back(rect);
	return rect;
}
StaticRectCollider2D* PhysicSimulation::createStaticRectCollider()
{
	StaticRectCollider2D* rect = new StaticRectCollider2D;
	m_statics.push_back(rect);
	m_colliders.push_back(rect);
	return rect;
}
}