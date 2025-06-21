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
	ImGuiLayer(PlatformWindow* window) : Layer(window), m_renderData(nullptr) {}
	~ImGuiLayer() {}
	struct Color {
		static const ImVec4 red;
		static const ImVec4 blue;
		static const ImVec4 dark;
		static const ImVec4 light;
	};
	static ImTextureID getTextureID(gfx::GraphicDevice* _device, gfx::DescriptorSetHandle _handle);
private:
	void onLayerCreate(Renderer* _renderer) override;
	void onLayerDestroy(Renderer* _renderer) override;

	void onLayerPreRender() override;
	void onLayerRender(aka::Renderer* _renderer, gfx::FrameHandle frame) override;
	void onLayerPostRender() override;
	void onLayerResize(uint32_t width, uint32_t height) override;
private:
	ImGuiRenderData* m_renderData;
};

#endif

};