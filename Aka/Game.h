#pragma once

#include "Core/Application.h"
#include "Core/Font.h"
#include "Core/Sprite.h"
#include "Platform/Window.h"
#include "Platform/IO/Image.h"
#include "Graphic/Batch.h"

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
	void frame() override;
	void update(Time::Unit deltaTime) override;
	void renderGUI();
	void render(GraphicBackend& backend) override;
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
	Entity* m_characterEntity;
	Entity* m_backgroundEntity;
	Entity* m_textEntity;
};

}

