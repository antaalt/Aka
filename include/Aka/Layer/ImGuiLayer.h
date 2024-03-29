#pragma once

#include <Aka/Core/Layer.h>

namespace aka {

#if defined(AKA_USE_IMGUI_LAYER)

class ImGuiLayer final : public Layer
{
public:
	ImGuiLayer() {}
	~ImGuiLayer() {}
private:
	void onLayerCreate() override;
	void onLayerDestroy() override;

	void onLayerFrame() override;
	void onLayerPresent() override;
};

#endif

};