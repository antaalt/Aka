#pragma once

#include <Aka/Core/Layer.h>

#include <atomic>

namespace aka {

#if defined(AKA_USE_IMGUI_LAYER)

class ImGuiLayer final : public Layer
{
public:
	ImGuiLayer();
	~ImGuiLayer();

	void onLayerAttach() override;
	void onLayerDetach() override;

	void onLayerFrame() override;
	void onLayerPresent() override;
};

#endif

};