# Aka Engine

Small 2D game engine, mainly built for personal educational purpose, to be easy to use and to extend. Its main target is pixel art game, and its pipeline is built around [Ogmo editor](https://ogmo-editor-3.github.io/) for level creation and [aseprite](https://www.aseprite.org/) for sprite creation.

Aka stand for red in japanese (赤) and there is no particular reason for this name, we just need one !

## Features
- Platform backend with GLFW 3.2
- Graphic backend with OpenGL 3.2 / DirectX 11
- Audio backend with RtAudio (mp3 support)
- Easily implement new window library / platform
- Event system for efficient communication
- UTF8 support
- Cross platform (not tested on apple)
- ECS system based on entt

## How to use
This basic example create a window and spin a square in the middle of the screen.
```cpp
struct Game :
	aka::Application,
	aka::EventListener<aka::KeyboardKeyDownEvent>
{
	aka::anglef rotation = aka::anglef::radian(0.f);
	void onCreate() override {}
	void onDestroy() override {}
	void onUpdate(aka::Time::Unit deltaTime) override {
		rotation += aka::anglef::radian(deltaTime.seconds());
	}
	void onRender() override {
		aka::Framebuffer::Ptr backbuffer = aka::GraphicBackend::backbuffer();
		backbuffer->clear(aka::color4f(0.f, 0.f, 1.f, 1.f));
		aka::vec2f size = aka::vec2f(300.f);
		aka::vec2f position = aka::vec2f(
			backbuffer->width() / 2.f - size.x / 2.f,
			backbuffer->height() / 2.f - size.y / 2.f
		);
		aka::mat3f transform = aka::mat3f::identity();
		transform *= aka::mat3f::translate(position);
		transform *= aka::mat3f::translate(0.5f * size);
		transform *= aka::mat3f::rotate(rotation);
		transform *= aka::mat3f::translate(-0.5f * size);
		transform *= aka::mat3f::scale(size);
		aka::Renderer2D::drawRect(
			transform,
			aka::vec2f(0.f),
			aka::vec2f(1.f),
			nullptr,
			aka::color4f(1.f, 0.f, 0.f, 1.f),
			0
		);
		aka::Renderer2D::render(backbuffer);
		aka::Renderer2D::clear();
	}
	void onReceive(const aka::KeyboardKeyDownEvent& event) override {
		if (event.key == aka::KeyboardKey::Escape)
			aka::EventDispatcher<aka::QuitEvent>::emit();
	}
};
int main()
{
	Game game;
	aka::Config cfg;
	cfg.width = 1280;
	cfg.height = 720;
	cfg.name = "Game";
	cfg.app = &game;
	aka::Application::run(cfg);
	return 0;
}
```

## Build
-   Run --recursive with git clone or git submodule init / update to get all dependencies.
-   Build using CMake. It has been tested on Windows (VS 2019) & Linux (GCC).

## RoadMap
-   More robust physic engine or use third party like [Box2D](https://box2d.org/)
-   Or simply add multiple shapes for colliders
-   ECS serialization
-   [Let's work on a game](https://github.com/antaalt/AkaGame) !
