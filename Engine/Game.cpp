#include "Game.h"

#include "Platform.h"
#include "Image.h"
#include "Input.h"
#include "Time.h"
#include "System.h"
#include "SpriteAnimatorComponent.h"
#include "WorldComponent.h"
#include "Collider2D.h"

#include <iostream>

namespace app {

SpriteAnimatorComponent debugSpriteAnimator;
Sprite debugSprite;
void createDebugSprite(GraphicBackend& backend)
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

// jump physic : https://2dengine.com/?p=platformers

struct PhysicWorld;

struct RigidBody
{
	friend struct PhysicWorld;

	RectCollider2D* collider = nullptr;
	vec2f position;
	vec2f size;
	float mass = 1.f;
private:
	vec2f velocity = vec2f(0.f);
	vec2f acceleration = vec2f(0.f);
};

struct PhysicWorld
{
	// update the physic simulation
	void update(float dt) {
		if (input::pressed(input::Key::Space))
		{
			bodies[0]->acceleration = vec2f(0.f, 0.f);
			bodies[0]->velocity = vec2f(0.f, 1.f);

			/*bodies[0]->acceleration = vec2f(0.f, 0.f);
			bodies[0]->velocity = vec2f(0.f, 0.f);
			bodies[0]->position = vec2f(192.f, 160.f);*/
		}
		if (input::pressed(input::Key::LeftCtrl))
		{
			bodies[0]->position = vec2f(192.f, 160.f);
		}
		// F = m*(G*Mearth/(Rearth*Rearth))
		// Mearth = 5.98 * 10^24kg
		// Rearth = 6.38*10^6m
		// G = 6.67259* 10^-11 Nm^2/kg^2
		// F = ma
		Time::unit now = Time::now();
		// dt is the timestep
		while (lastTick + dt < now)
		{
			// Update velocity vectors
			vec2f force = vec2f(0.f, -9.81f);
			vec2f maxVelocity(50.f);
			for (RigidBody* rigid : bodies)
			{
				rigid->acceleration += (force / rigid->mass) * dt;
				rigid->velocity += rigid->acceleration * dt;
				rigid->velocity = min(rigid->velocity, maxVelocity);
			}
			// Move dynamic objects
			for (RigidBody* rigid : bodies)
			{
				// Move rigid
				rigid->position += rigid->velocity;
				// Check collisions with static objects
				for (RectCollider2D* collider : colliders)
				{
					rigid->collider->position = rigid->position;
					Collision c = overlap(*rigid->collider, *collider);
					if (c.collided)
					{
						// Adjust velocity
						// Get normal 
						vec2f normal = vec2f::normalize(c.separation);
						// Get relative velocity
						vec2f velocity = rigid->velocity; // substract by second object velocity if it has one
						// Get penetration speed
						float ps = vec2f::dot(velocity, normal);
						// objects moving towards each other ?
						if (ps <= 0.f)
						{
							// Move the rigid to avoid overlapping.
							rigid->position += c.separation;
						}
						// Get penetration component
						vec2f p = normal * ps;
						// tangent component
						vec2f t = velocity - p;
						// Restitution
						float r = 1.f + max<float>(0.1f, 0.f); // max bouncing value of object a & b
						// Friction
						float f = min<float>(0.1f, 1.f); // max friction value of object a & b
						// Change the velocity of shape a
						rigid->acceleration = vec2f(0);
						rigid->velocity = rigid->velocity - p * r + t * f;
					}
				}
				// Check collisions with other dynamic object
				for (RigidBody* otherRigid : bodies)
				{
					// Skip self intersection
					if (otherRigid == rigid)
						continue;
					Collision c = overlap(*otherRigid->collider, *rigid->collider);
					if (c.collided)
					{
						// HAAA handle collision
						// Move one of the object to avoid overlapping.
						// Adjust velocity
					}
				}
			}
			lastTick += Time::unit(dt * 1000.f);
		}
	}
	void render(GraphicBackend& backend) {
		// For debug
		for (RigidBody* rigid : bodies)
		{
			debugSprite.position = rigid->position;
			debugSprite.size = rigid->size;
			debugSpriteAnimator.render(backend);
		}
		/*for (RectCollider2D* collider : colliders)
		{
			debugSprite.position = collider->position;
			debugSprite.size = collider->size;
			debugSpriteAnimator.render(backend);
		}*/
	}
	Time::unit lastTick = 0;
	std::vector<RigidBody*> bodies;
	std::vector<RectCollider2D*> colliders;
};
// Collider is a component, animator is a component

/*struct Player : Object {
	void create() {
		components.push_back(new PhysicComponent);
		components.push_back(new ColliderComopnent);
		components.push_back(new SpriteAnimatorComponent);
	}
	std::vector<Component*> components;
};*/

GLuint framebufferID;
Sprite character;
Sprite background;
SpriteAnimatorComponent characterComponent;
SpriteAnimatorComponent backgroundComponent;
Texture* renderTarget;
FontRenderer* fontRenderer;
Font font24, font48, font96;
WorldComponent worldComponent;

const uint32_t BACKGROUND_WIDTH = 320, BACKGROUND_HEIGHT = 180;
const uint32_t CHAR_WIDTH = 16, CHAR_HEIGHT = 32;
GLenum error = GL_NO_ERROR;

vec2f charPosition;

PhysicWorld physicWorld;


void Game::initialize(Window& window, GraphicBackend& backend)
{
	{
		// INIT world
		worldComponent.loadWorld(Asset::path("levels/world.ogmo"));
		worldComponent.loadLevel(Asset::path("levels/level0.json"));
		worldComponent.create(backend);
	}
	{
		// INIT TEXTURE BACKGROUND
		Image image = Image::load(Asset::path("textures/background/background.png"));
		ASSERT(image.width == BACKGROUND_WIDTH, "incorrect width");
		ASSERT(image.height == BACKGROUND_HEIGHT, "incorrect height");

		Sprite::Animation animation;
		animation.name = "default";
		animation.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 500));
		background.animations.push_back(animation);
		background.position = vec2f(0);
		background.size = vec2f(BACKGROUND_WIDTH, BACKGROUND_HEIGHT);
		background.rotation = radianf(0);
		
		backgroundComponent.set(&background);
		backgroundComponent.create(backend);
		backgroundComponent.play("default");
	}
	{
		// INIT SPRITE CHARACTER
		// TODO move initialisation in SpriteAnimator & parse ase sprite directly ?
		// TODO load gif ?
		Sprite::Animation animation;
		animation.name = "idle";
		{
			Image image = Image::load(Asset::path("textures/player/player1.png"));
			animation.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 500));
		}
		{
			Image image = Image::load(Asset::path("textures/player/player2.png"));
			animation.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 500));
		}
		{
			Image image = Image::load(Asset::path("textures/player/player3.png"));
			animation.frames.push_back(Sprite::Frame::create(backend.createTexture(image.width, image.height, image.bytes.data()), 500));
		}
		character.animations.push_back(animation);
		character.position = vec2f(BACKGROUND_WIDTH / 2.f, BACKGROUND_HEIGHT / 2.f);
		character.size = vec2f(CHAR_WIDTH, CHAR_HEIGHT);
		character.rotation = degreef(0);
		characterComponent.set(&character);
		characterComponent.create(backend);
		characterComponent.play("idle");
	}

	// INIT FRAMEBUFFER RENDER TARGET
	renderTarget = backend.createTexture(BACKGROUND_WIDTH, BACKGROUND_HEIGHT, nullptr);

	// INIT FONT
	fontRenderer = backend.createFontRenderer();
	font24 = fontRenderer->createFont(Asset::path("font/Espera-Bold.ttf"), 24);
	font48 = fontRenderer->createFont(Asset::path("font/Espera-Bold.ttf"), 48);
	font96 = fontRenderer->createFont(Asset::path("font/Espera-Bold.ttf"), 96);

	// INIT FRAMEBUFFER
	glGenFramebuffers(1, &framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget->getID(), 0);
	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not created");

	window.setSizeLimits(BACKGROUND_WIDTH, BACKGROUND_HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);

	createDebugSprite(backend);
	{
		RigidBody* rigid = new RigidBody;
		rigid->position = vec2f(192, 160);
		rigid->size = vec2f(16);
		rigid->collider = new RectCollider2D;
		rigid->collider->position = rigid->position;
		rigid->collider->size = rigid->size;
		physicWorld.bodies.push_back(rigid);
	}
	{
		RectCollider2D *collider = new RectCollider2D;
		collider->position = vec2f(192, 0);
		collider->size = vec2f(128, 96);
		physicWorld.colliders.push_back(collider);
	}
	{
		RectCollider2D* collider = new RectCollider2D;
		collider->position = vec2f(0);
		collider->size = vec2f(320, 80);
		physicWorld.colliders.push_back(collider);
	}
	physicWorld.lastTick = Time::now();
}

void Game::destroy(GraphicBackend& backend)
{
	renderTarget->destroy();
	characterComponent.destroy(backend);
	worldComponent.destroy(backend);
	fontRenderer->destroyFont(font24);
	fontRenderer->destroyFont(font48);
	fontRenderer->destroyFont(font96);
	fontRenderer->destroy();
	background.animations[0].frames[0].texture->destroy();
	for(Sprite::Animation &animation: character.animations)
		for (Sprite::Frame& frame : animation.frames)
			frame.texture->destroy();
	glDeleteFramebuffers(1, &framebufferID);
}


void Game::update(GraphicBackend& backend)
{
	//character.position.x += input::pressed(input::Key::D) - input::pressed(input::Key::Q);
	//character.position.y += input::pressed(input::Key::ArrowUp) - input::pressed(input::Key::ArrowDown);
	//character.rotation = degreef(Time::now() / 100.f);
	characterComponent.update();
	backgroundComponent.update();
	if (input::pressed(input::Button::Button1))
	{
		worldComponent.getCurrentLevel().offset.x += (int32_t)input::delta().x;
		worldComponent.getCurrentLevel().offset.y += (int32_t)-input::delta().y;
	}
	
	physicWorld.bodies[0]->position.x += input::pressed(input::Key::D) - input::pressed(input::Key::Q) + input::pressed(input::Key::ArrowRight) - input::pressed(input::Key::ArrowLeft);
	physicWorld.bodies[0]->position.y += input::pressed(input::Key::Z) - input::pressed(input::Key::S) + input::pressed(input::Key::ArrowUp) - input::pressed(input::Key::ArrowDown);
	// Update physic after moving manually objects
	physicWorld.update(0.01f);
}
// TODO
// - Hide framebuffer impl
void Game::render(GraphicBackend& backend)
{
	backend.viewport(0, 0, BACKGROUND_WIDTH, BACKGROUND_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	backend.clear(color4f(0.f, 0.f, 0.f, 1.f));
	// draw background
	backgroundComponent.render(backend);
	worldComponent.render(backend);
	characterComponent.render(backend);

	physicWorld.render(backend);

	// Blit to main buffer
	uint32_t widthRatio = screenWidth() / BACKGROUND_WIDTH;
	uint32_t heightRatio = screenHeight() / BACKGROUND_HEIGHT;
	uint32_t ratio = min(widthRatio, heightRatio);
	uint32_t scaledWidth = ratio * BACKGROUND_WIDTH;
	uint32_t scaledHeight = ratio * BACKGROUND_HEIGHT;
	uint32_t w = (screenWidth() - scaledWidth) / 2;
	uint32_t h = (screenHeight() - scaledHeight) / 2;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	backend.viewport(0, 0, screenWidth(), screenHeight());
	backend.clear(color4f(0.f, 0.f, 0.f, 1.f));
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, BACKGROUND_WIDTH, BACKGROUND_HEIGHT, w, h, screenWidth() - w, screenHeight() - h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Draw text
	//backend.viewport(0, 0, screenWidth(), screenHeight());
	//fontRenderer->viewport(0, 0, screenWidth(), screenHeight());
	static float scale = 1.f;
	scale += input::scroll().y;
	const char* str = "Hello world !";
	vec2i size24 = font24.size(str);// *(int)scale;
	vec2i size48 = font48.size(str);// *(int)scale;
	vec2i size96 = font96.size(str);// *(int)scale;
	//fontRenderer->render(font48, str, (float)((int)screenWidth() / 2 - size24.x / 2), (float)((int)screenHeight() / 2 - size48.y / 2), 1.f, color3f(0.1f, 0.1f, 0.1f));
	//fontRenderer->render(font24, str, 10, (float)((int)screenHeight() / 2 - size24.y / 2), 1.f, color3f(0.1f, 0.1f, 0.1f));
	//fontRenderer->render(font96, str, (float)((int)screenWidth() / 2 - size96.x / 2) + size96.x + 10, (float)((int)screenHeight() / 2 - size96.y / 2), 1.f, color3f(0.1f, 0.1f, 0.1f));

	ASSERT((error = glGetError()) == GL_NO_ERROR, "");
}

}