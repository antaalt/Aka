#pragma once

#include <Aka/Core/Layer.h>

namespace aka {

#if defined(AKA_USE_IMGUI_LAYER)

struct ImGuiRenderData;

class ImGuiLayer final : public Layer
{
public:
	ImGuiLayer() : m_renderData(nullptr) {}
	~ImGuiLayer() {}
private:
	void onLayerCreate() override;
	void onLayerDestroy() override;

	void onLayerFrame() override;
	void onLayerRender(gfx::Frame* frame) override;
	void onLayerPresent() override;
	void onLayerResize(uint32_t width, uint32_t height) override;
private:
	ImGuiRenderData* m_renderData;
};

#endif

};