#pragma once

#include "../Resources.h"

#define USE_IMGUI

#include <sstream>
#if defined(USE_IMGUI)
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#if defined(AKA_USE_OPENGL)
#include <examples/imgui_impl_opengl3.h>
#endif
#if defined(AKA_USE_D3D11)
#include <examples/imgui_impl_dx11.h>
#endif
#endif

namespace aka {

class GUIWidget
{
public:
	virtual void update() {}
	virtual void draw(World& world, Resources& resources) {}
};

class GUI
{
public:
	GUI() {}
	~GUI() {
		for (GUIWidget* widget : m_widgets)
			delete widget;
	}
	void initialize();
	void destroy();
	void update() {
		for (GUIWidget *widget : m_widgets)
			widget->update();
	}
	void draw(World& world, Resources& resources) {
		if (m_visible)
			for (GUIWidget* widget : m_widgets)
				widget->draw(world, resources);
	}
	void frame();
	void render();
	bool isVisible() { return m_visible; }
	void setVisible(bool visible) { m_visible = visible; }

	void add(GUIWidget* widget) { m_widgets.push_back(widget); }

	bool focused() const { const ImGuiIO& io = ImGui::GetIO(); return io.WantCaptureMouse; }
private:
	bool m_visible = true;
	std::vector<GUIWidget*> m_widgets;
};

}