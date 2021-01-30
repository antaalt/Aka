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
		/*ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;
		colors[ImGuiCol_Text] = ImVec4(0.31f, 0.25f, 0.24f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_ChildWindowBg] = ImVec4(0.68f, 0.68f, 0.68f, 0.00f);
		colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.19f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.62f, 0.70f, 0.72f, 0.56f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95f, 0.33f, 0.14f, 0.47f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.97f, 0.31f, 0.13f, 0.81f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.42f, 0.75f, 1.00f, 0.53f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.65f, 0.80f, 0.20f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.40f, 0.62f, 0.80f, 0.15f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.39f, 0.64f, 0.80f, 0.30f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.67f, 0.80f, 0.59f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.48f, 0.53f, 0.67f);
		colors[ImGuiCol_ComboBg] = ImVec4(0.89f, 0.98f, 1.00f, 0.99f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.38f, 0.37f, 0.37f, 0.91f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.31f, 0.47f, 0.99f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(1.00f, 0.79f, 0.18f, 0.78f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.42f, 0.82f, 1.00f, 0.81f);
		colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.86f);
		colors[ImGuiCol_Header] = ImVec4(0.73f, 0.80f, 0.86f, 0.45f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.88f, 0.94f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.46f, 0.84f, 0.90f, 1.00f);
		colors[ImGuiCol_CloseButton] = ImVec4(0.41f, 0.75f, 0.98f, 0.50f);
		colors[ImGuiCol_CloseButtonHovered] = ImVec4(1.00f, 0.47f, 0.41f, 0.60f);
		colors[ImGuiCol_CloseButtonActive] = ImVec4(1.00f, 0.16f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.99f, 0.54f, 0.43f);
		colors[ImGuiCol_TooltipBg] = ImVec4(0.82f, 0.92f, 1.00f, 0.90f);
		style.Alpha = 1.0f;
		style.WindowFillAlphaDefault = 1.0f;
		style.FrameRounding = 4;*/
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