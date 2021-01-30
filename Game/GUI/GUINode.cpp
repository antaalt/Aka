#include "GUINode.h"

namespace aka {

void GUI::initialize()
{
#if defined(USE_IMGUI)
	{
		// IMGUI
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

#if defined(AKA_USE_OPENGL)
		ImGui_ImplGlfw_InitForOpenGL(PlatformBackend::getGLFW3Handle(), true);

		float glLanguageVersion = (float)atof((char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		std::stringstream ss;
		ss << "#version " << (GLuint)(100.f * glLanguageVersion) << std::endl;
		ImGui_ImplOpenGL3_Init(ss.str().c_str());
#else
		ImGui_ImplGlfw_InitForVulkan(PlatformBackend::getGLFW3Handle(), true);
		ImGui_ImplDX11_Init(GraphicBackend::getD3D11Device(), GraphicBackend::getD3D11DeviceContext());
#endif
		ImGui::StyleColorsClassic();
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;
        auto ImLerp = [](const ImVec4& a, const ImVec4& b, float t)->ImVec4 {
            return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
        };
        ImVec4 red = ImVec4(0.93f, 0.04f, 0.26f, 1.f);
        ImVec4 blue = ImVec4(0.01f, 0.47f, 0.96f, 1.f);
        ImVec4 dark = ImVec4(0.1f, 0.1f, 0.1f, 1.f);
        ImVec4 light = ImVec4(0.9f, 0.9f, 0.9f, 1.f);
        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.9f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        colors[ImGuiCol_FrameBg] = dark;
        colors[ImGuiCol_FrameBgHovered] = ImLerp(dark, light, 0.5f);
        colors[ImGuiCol_FrameBgActive] = red;

        colors[ImGuiCol_TitleBg] = ImLerp(red, dark, 0.8f);
        colors[ImGuiCol_TitleBgActive] = red;
        colors[ImGuiCol_TitleBgCollapsed] = ImLerp(red, dark, 0.9f);

        colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);

        colors[ImGuiCol_CheckMark] = red;

        colors[ImGuiCol_SliderGrab] = red;
        colors[ImGuiCol_SliderGrabActive] = blue;
        // Header
        colors[ImGuiCol_Header] = red;
        colors[ImGuiCol_HeaderHovered] = ImLerp(red, light, 0.1f);
        colors[ImGuiCol_HeaderActive] = ImLerp(red, light, 0.3f);
        // Button
        colors[ImGuiCol_Button] = colors[ImGuiCol_Header];
        colors[ImGuiCol_ButtonHovered] = colors[ImGuiCol_HeaderHovered];
        colors[ImGuiCol_ButtonActive] = colors[ImGuiCol_HeaderActive];
        // Separator
        colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        // Resize
        colors[ImGuiCol_ResizeGrip] = colors[ImGuiCol_Header];
        colors[ImGuiCol_ResizeGripHovered] = colors[ImGuiCol_HeaderHovered];
        colors[ImGuiCol_ResizeGripActive] = colors[ImGuiCol_HeaderActive];
        // Tab
        colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
        colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
        colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
        colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
        colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
        // Plot
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

        colors[ImGuiCol_TextSelectedBg] = red;

        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);

        colors[ImGuiCol_NavHighlight] = red;
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.4f);

		style.Alpha = 1.0f;
		style.FrameRounding = 2;
        style.WindowRounding = 4;
        style.PopupRounding = 2;
        style.ChildRounding = 2;
        style.FrameBorderSize = 1.f;
        style.WindowBorderSize = 1.f;
        style.PopupBorderSize = 1.f;
	}
#endif
}

void GUI::destroy()
{
#if defined(USE_IMGUI)
#if defined(AKA_USE_OPENGL)
	ImGui_ImplOpenGL3_Shutdown();
#else
	ImGui_ImplDX11_Shutdown();
#endif
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
#endif
}

void GUI::frame()
{
#if defined(USE_IMGUI)
	// Start the Dear ImGui frame
#if defined(AKA_USE_OPENGL)
	ImGui_ImplOpenGL3_NewFrame();
#else
	ImGui_ImplDX11_NewFrame();
#endif
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
#endif
}

void GUI::render()
{
#if defined(USE_IMGUI)
	ImGui::Render();
#if defined(AKA_USE_OPENGL)
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#else
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif
#endif
}

};