#pragma once

#include "Core/Application.h"
#include "Core/Font.h"
#include "Platform/Window.h"
#include "Platform/IO/Image.h"
#include "Graphic/Batch.h"
#include "System/PhysicSystem.h"
#include "System/AnimatorSystem.h"
#include "System/TileMapSystem.h"
#include "System/TileRenderSystem.h"
#include "System/TextRenderSystem.h"
#include "System/CameraSystem.h"

namespace aka {

// Module for third parties like ImGui ?
struct Module
{
	virtual void initialize(Window& window, GraphicBackend& backend) = 0;
	virtual void destroy(GraphicBackend& backend) = 0;
	virtual void update(GraphicBackend& backend) = 0;
	virtual void render(GraphicBackend& backend) = 0;
};

class Game : public Application
{
public:
	void initialize(Window& window, GraphicBackend& backend) override;
	void destroy(GraphicBackend& backend) override;
	void update(Time::Unit deltaTime) override;
	void render(GraphicBackend& backend) override;
private:
	// Resources
	Font m_font;
	Sprite m_character;
	Sprite m_background;
	Sprite m_colliderSprite;
	// Rendering
	Batch m_batch;
	Framebuffer::Ptr m_framebuffer;
	// Entity
	Entity* m_characterEntity;
	Entity* m_backgroundEntity;
	Entity* m_textEntity;
	// Systems
	PhysicSystem* m_physicSystem;
	AnimatorSystem* m_animatorSystem;
	TileMapSystem* m_tileMapSystem;
	TileSystem* m_tileSystem;
	CameraSystem* m_cameraSystem;
	TextRenderSystem* m_textRenderingSystem;
};

}

