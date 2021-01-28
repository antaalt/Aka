#pragma once

#include "../Core/Application.h"
#include "../Core/Font.h"
#include "../Core/Sprite.h"
#include "../OS/Image.h"
#include "../Graphic/Batch.h"

namespace aka {

class Game : public Application
{
public:
	void initialize() override;
	void destroy() override;
	void frame() override;
	void update(Time::Unit deltaTime) override;
	void renderGUI();
	void render() override;
	bool running() override;
private:
	// UI
	bool m_displayUI = true;
	// Resources
	std::vector<std::shared_ptr<Font>> m_fonts;
	std::vector<std::shared_ptr<Sprite>> m_sprites;
	// Rendering
	Batch m_batch;
	Framebuffer::Ptr m_framebuffer;
	// Entity
	World m_world;
	Entity* m_cameraEntity;
	Entity* m_playerEntity;
	Entity* m_backgroundEntity;
	Entity* m_textEntity;
};

}

