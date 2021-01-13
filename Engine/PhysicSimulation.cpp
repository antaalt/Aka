#include "PhysicSimulation.h"

#include "Image.h"

namespace app {

SpriteAnimatorComponent debugSpriteAnimator;
Sprite debugSprite;

PhysicSimulation::PhysicSimulation(float timestep) :
	timestep(timestep),
	lastTick(Time::now()),
	running(false),
	renderColliders(true)
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
	running = true;
	lastTick = Time::now();
}

void PhysicSimulation::pause()
{
	running = false;
}

void PhysicSimulation::update()
{
	if (!running)
		return;

	const Time::unit now = Time::now();
	const vec2f force = vec2f(0.f, -9.81f);
	float dt = timestep;
	// dt is the timestep
	while (lastTick + dt < now)
	{
		// Update velocity vectors
		for (DynamicCollider2D* dynamic : dynamics)
		{
			dynamic->addForce(force, dt);
		}
		// Move dynamic objects
		for (DynamicCollider2D* dynamic : dynamics)
		{
			// Move rigid
			dynamic->move(dt);
			// Check collisions with static objects
			for (StaticCollider2D* collider : statics)
			{
				Collision c = dynamic->overlaps(*collider);
				if (c.collided)
				{
					dynamic->resolve(c.separation, *collider);
				}
			}
			// Check collisions with other dynamic object
			for (DynamicCollider2D* otherDynamic : dynamics)
			{
				// Skip self intersection
				if (otherDynamic == dynamic)
					continue;
				Collision c = dynamic->overlaps(*otherDynamic);
				if (c.collided)
				{
					dynamic->resolve(c.separation, *otherDynamic);
				}
			}
		}
		lastTick += static_cast<Time::unit>(dt * 1000.f);
	}
}
void PhysicSimulation::render(const Camera2D &camera, GraphicBackend& backend)
{
	if (!renderColliders)
		return;
	// For debug
	for (DynamicCollider2D* dynamic : dynamics)
	{
		debugSprite.position = dynamic->getShape()->getPosition();
		debugSprite.size = dynamic->getShape()->getSize();
		debugSpriteAnimator.render(camera, backend);
	}
	for (StaticCollider2D* statics : statics)
	{
		debugSprite.position = statics->getShape()->getPosition();
		debugSprite.size = statics->getShape()->getSize();
		debugSpriteAnimator.render(camera, backend);
	}
}
DynamicRectCollider2D* PhysicSimulation::createDynamicRectCollider()
{
	DynamicRectCollider2D *rect = new DynamicRectCollider2D;
	dynamics.push_back(rect);
	colliders.push_back(rect);
	return rect;
}
StaticRectCollider2D* PhysicSimulation::createStaticRectCollider()
{
	StaticRectCollider2D* rect = new StaticRectCollider2D;
	statics.push_back(rect);
	colliders.push_back(rect);
	return rect;
}
}