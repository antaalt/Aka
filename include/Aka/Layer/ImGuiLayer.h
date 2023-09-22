#pragma once

#include <Aka/Core/Layer.h>


#if defined(AKA_USE_IMGUI_LAYER)
#include <imgui.h>
#include <imguizmo.h>
#include <IconsFontAwesome6.h>
#endif

namespace aka {

#if defined(AKA_USE_IMGUI_LAYER)

struct ImGuiRenderData;

class ImGuiLayer final : public Layer
{
public:
	ImGuiLayer() : m_renderData(nullptr) {}
	~ImGuiLayer() {}
	struct Color {
		static const ImVec4 red;
		static const ImVec4 blue;
		static const ImVec4 dark;
		static const ImVec4 light;
	};
	static ImTextureID getTextureID(gfx::GraphicDevice* _device, gfx::DescriptorSetHandle _handle);
private:
	void onLayerCreate(gfx::GraphicDevice* _device) override;
	void onLayerDestroy(gfx::GraphicDevice* _device) override;

	void onLayerFrame() override;
	void onLayerRender(aka::gfx::GraphicDevice* _device, gfx::Frame* frame) override;
	void onLayerPresent() override;
	void onLayerResize(uint32_t width, uint32_t height) override;
private:
	ImGuiRenderData* m_renderData;
};

#endif

};