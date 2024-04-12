#include <Aka/Layer/ImGuiLayer.h>

#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Core/Application.h>
#include <Aka/OS/OS.h>

#if defined(AKA_USE_IMGUI_LAYER)

#include <backends/imgui_impl_glfw.h>

#include "Fonts/FontAwesomeRegular400.hpp"
#include "Fonts/FontAwesomeSolid900.hpp"
#include "Fonts/RobotoRegular.hpp"

// TODO rewrite imgui backend with aka
#if defined(AKA_USE_OPENGL)
#include <GL/glew.h>
#include <GL/gl.h>
#include <backends/imgui_impl_opengl3.h>
#include <sstream>
#endif
#if defined(AKA_USE_D3D11)
#include <d3d11.h>
#include <backends/imgui_impl_dx11.h>
#include "Graphic/D3D11/D3D11Context.h"
#include "Graphic/D3D11/D3D11Device.h"
#include "Graphic/D3D11/D3D11Backbuffer.h"
#endif
#if defined(AKA_USE_VULKAN)
#include "Graphic/Vulkan/VulkanContext.h"
#include "Graphic/Vulkan/VulkanGraphicDevice.h"
#include <backends/imgui_impl_vulkan.h>
#endif
#include "Platform/GLFW3/PlatformGLFW3.h"

namespace aka {

#if defined(AKA_USE_VULKAN)
struct ImGuiRenderData
{
	VkDescriptorPool descriptorPool;
	gfx::RenderPassHandle renderPass;
	gfx::BackbufferHandle backbuffer;
};
#else
struct ImGuiRenderData {};
#endif

const ImVec4 ImGuiLayer::Color::red = ImVec4(0.93f, 0.04f, 0.26f, 1.f);
const ImVec4 ImGuiLayer::Color::blue = ImVec4(0.01f, 0.47f, 0.96f, 1.f);
const ImVec4 ImGuiLayer::Color::dark = ImVec4(0.1f, 0.1f, 0.1f, 1.f);
const ImVec4 ImGuiLayer::Color::light = ImVec4(0.9f, 0.9f, 0.9f, 1.f);

ImTextureID ImGuiLayer::getTextureID(gfx::GraphicDevice* _device, gfx::DescriptorSetHandle _texture)
{
	switch (_device->getApi())
	{
	case gfx::GraphicAPI::Vulkan:
		return ImTextureID{ reinterpret_cast<const gfx::VulkanDescriptorSet*>(_device->get(_texture))->vk_descriptorSet };
	default:
	case gfx::GraphicAPI::DirectX12:
		AKA_NOT_IMPLEMENTED;
		return ImTextureID{};
	}
}

void ImGuiLayer::onLayerCreate(Renderer* _renderer)
{
	AKA_ASSERT(m_renderData == nullptr, "");
	m_renderData = new ImGuiRenderData;
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls


	PlatformGLFW3* platform = reinterpret_cast<PlatformGLFW3*>(Application::app()->platform());
#if defined(AKA_USE_OPENGL)
	ImGui_ImplGlfw_InitForOpenGL(platform->getGLFW3Handle(), true);

	float glLanguageVersion = (float)atof((char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
	std::stringstream ss;
	ss << "#version " << (GLuint)(100.f * glLanguageVersion) << std::endl;
	ImGui_ImplOpenGL3_Init(ss.str().c_str());
#elif defined(AKA_USE_D3D11)
	D3D11Device* device = reinterpret_cast<D3D11Device*>(Application::graphic());
	ImGui_ImplGlfw_InitForVulkan(platform->getGLFW3Handle(), true);
	ImGui_ImplDX11_Init(device->device(), device->context());
#elif defined(AKA_USE_VULKAN)
	gfx::VulkanGraphicDevice* device = reinterpret_cast<gfx::VulkanGraphicDevice*>(_renderer->getDevice());

	{ // Custom descriptor pool for imgui
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		VK_CHECK_RESULT(vkCreateDescriptorPool(device->getVkDevice(), &pool_info, nullptr, &m_renderData->descriptorPool));
		//gfx::VulkanProgram::createVkDescriptorSet()
	}
	{
		m_renderData->renderPass = device->createBackbufferRenderPass(gfx::AttachmentLoadOp::Load, gfx::AttachmentStoreOp::Store, gfx::ResourceAccessType::Present);
		m_renderData->backbuffer = device->createBackbuffer(m_renderData->renderPass);
	}

	ImGui_ImplGlfw_InitForVulkan(platform->getGLFW3Handle(), true);
	ImGui_ImplVulkan_InitInfo info{};
	info.Instance = device->getVkInstance();
	info.PhysicalDevice = device->getVkPhysicalDevice();
	info.Device = device->getVkDevice();
	info.QueueFamily = device->getVkQueueIndex(gfx::QueueType::Graphic);
	info.Queue = device->getVkQueue(gfx::QueueType::Graphic);
	info.PipelineCache = VK_NULL_HANDLE;
	info.DescriptorPool = m_renderData->descriptorPool;
	info.MinImageCount = 2; // >= 2
	info.ImageCount = static_cast<uint32_t>(device->getSwapchainImageCount()); // >= MinImageCount
	info.CheckVkResultFn = [](VkResult err) {
		VK_CHECK_RESULT(err);
	};

	ImGui_ImplVulkan_Init(&info, device->getVk<gfx::VulkanRenderPass>(m_renderData->renderPass)->vk_renderpass);

#endif


	// --- Font
	float baseFontSize = 22.0f; // 13.0f is the size of the default font. Change to the font size you use.
	float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

	ImFontConfig config;
	//config.GlyphMinAdvanceX = iconFontSize;
	config.PixelSnapH = true;
	config.FontDataOwnedByAtlas = false; // We keep ownership of given data.
	{
		//io.Fonts->AddFontDefault();
		Vector<unsigned char> data(font::RobotoRegular, sizeof(font::RobotoRegular));
		ImFont* iconFont = io.Fonts->AddFontFromMemoryTTF(data.data(), sizeof(font::RobotoRegular), iconFontSize, &config);
		AKA_ASSERT(iconFont != nullptr, "Roboto font not loaded");
	}
	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	config.MergeMode = true;
	{
		// For some reason, ImGui does not support const pointer, so copy it to non const...
		Vector<unsigned char> data(font::FontAwesomeRegular400, sizeof(font::FontAwesomeRegular400));
		ImFont* iconFont = io.Fonts->AddFontFromMemoryTTF(data.data(), sizeof(font::FontAwesomeRegular400), iconFontSize, &config, icon_ranges);
		AKA_ASSERT(iconFont != nullptr, "Icon font not loaded");
	}
	{
		// For some reason, ImGui does not support const pointer, so copy it to non const...
		Vector<unsigned char> data(font::FontAwesomeSolid900, sizeof(font::FontAwesomeSolid900));
		ImFont* iconFont = io.Fonts->AddFontFromMemoryTTF(data.data(), sizeof(font::FontAwesomeSolid900), iconFontSize, &config, icon_ranges);
		AKA_ASSERT(iconFont != nullptr, "Icon font not loaded");
	}
	

	// --- Style
	ImGui::StyleColorsClassic();
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;
	auto ImLerp = [](const ImVec4& a, const ImVec4& b, float t)->ImVec4 {
		return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
		};
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.9f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	colors[ImGuiCol_FrameBg] = Color::dark;
	colors[ImGuiCol_FrameBgHovered] = ImLerp(Color::dark, Color::light, 0.5f);
	colors[ImGuiCol_FrameBgActive] = Color::red;

	colors[ImGuiCol_TitleBg] = ImLerp(Color::red, Color::dark, 0.8f);
	colors[ImGuiCol_TitleBgActive] = Color::red;
	colors[ImGuiCol_TitleBgCollapsed] = ImLerp(Color::red, Color::dark, 0.9f);

	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);

	colors[ImGuiCol_CheckMark] = Color::red;

	colors[ImGuiCol_SliderGrab] = Color::red;
	colors[ImGuiCol_SliderGrabActive] = Color::blue;
	// Header
	colors[ImGuiCol_Header] = Color::red;
	colors[ImGuiCol_HeaderHovered] = ImLerp(Color::red, Color::light, 0.1f);
	colors[ImGuiCol_HeaderActive] = ImLerp(Color::red, Color::light, 0.3f);
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
	colors[ImGuiCol_Tab] = ImLerp(Color::red, Color::dark, 0.80f);
	colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_TabActive] = Color::red;
	colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
	colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
	// Plot
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

	colors[ImGuiCol_TableHeaderBg] = Color::red;
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);

	colors[ImGuiCol_TextSelectedBg] = Color::red;

	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);

	colors[ImGuiCol_NavHighlight] = Color::red;
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

void ImGuiLayer::onLayerDestroy(Renderer* _renderer)
{
	gfx::VulkanGraphicDevice * vk_device = reinterpret_cast<gfx::VulkanGraphicDevice*>(_renderer->getDevice());
#if defined(AKA_USE_OPENGL)
	ImGui_ImplOpenGL3_Shutdown();
#elif defined(AKA_USE_D3D11)
	ImGui_ImplDX11_Shutdown();
#elif defined(AKA_USE_VULKAN)
	ImGui_ImplVulkan_Shutdown();
#endif
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	vkDestroyDescriptorPool(vk_device->getVkDevice(), m_renderData->descriptorPool, nullptr);
	vk_device->destroy(m_renderData->renderPass);
	vk_device->destroy(m_renderData->backbuffer);
	delete m_renderData;
}

void ImGuiLayer::onLayerPreRender()
{
	// Start the Dear ImGui frame
#if defined(AKA_USE_OPENGL)
	ImGui_ImplOpenGL3_NewFrame();
#elif defined(AKA_USE_D3D11)
	ImGui_ImplDX11_NewFrame();
#elif defined(AKA_USE_VULKAN)
	if (!ImGui::GetIO().Fonts->TexID)
	{
		gfx::VulkanGraphicDevice* device = reinterpret_cast<gfx::VulkanGraphicDevice*>(Application::app()->graphic());
		VkCommandBuffer cmdBuffer = gfx::VulkanCommandList::createSingleTime("Create ImGui fonts", device->getVkDevice(), device->getVkCommandPool(gfx::QueueType::Graphic));
		ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
		gfx::VulkanCommandList::endSingleTime(device->getVkDevice(), device->getVkCommandPool(gfx::QueueType::Graphic), cmdBuffer, device->getVkQueue(gfx::QueueType::Graphic));

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
	ImGui_ImplVulkan_NewFrame(); // Nothing done
#endif
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void ImGuiLayer::onLayerRender(aka::Renderer* _renderer, gfx::FrameHandle frame)
{
	gfx::CommandList* cmd = _renderer->getDevice()->getGraphicCommandList(frame);
	gfx::VulkanCommandList* vk_cmd = reinterpret_cast<gfx::VulkanCommandList*>(cmd);
	ImGui::Render();
#if defined(AKA_USE_OPENGL)
	// TODO do not enforce backbuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#elif defined(AKA_USE_D3D11)
	// TODO do not enforce backbuffer
	GraphicDevice* device = Application::graphic();
	D3D11Backbuffer* backbuffer = (D3D11Backbuffer*)device->backbuffer().get();
	backbuffer->bind();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#elif defined(AKA_USE_VULKAN)
	// TODO do not enforce backbuffer
	gfx::FramebufferHandle framebuffer = _renderer->getDevice()->get(m_renderData->backbuffer, frame);
	const gfx::Framebuffer* fb = _renderer->getDevice()->get(framebuffer);
	{
		gfx::ScopedCmdMarker marker(cmd, "ImGui", &Color::red.x);
		cmd->transition(fb->colors[0].texture, gfx::ResourceAccessType::Attachment, gfx::ResourceAccessType::Attachment);
		cmd->transition(fb->depth.texture, gfx::ResourceAccessType::Attachment, gfx::ResourceAccessType::Attachment);
		cmd->beginRenderPass(m_renderData->renderPass, framebuffer, gfx::ClearState{});
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vk_cmd->getVkCommandBuffer());
		cmd->endRenderPass();
	}
#endif
}

void ImGuiLayer::onLayerPostRender()
{
}

void ImGuiLayer::onLayerResize(uint32_t width, uint32_t height)
{
	gfx::GraphicDevice* device = Application::app()->graphic();
	device->destroy(m_renderData->backbuffer);
	device->destroy(m_renderData->renderPass);
	m_renderData->renderPass = device->createBackbufferRenderPass(gfx::AttachmentLoadOp::Load, gfx::AttachmentStoreOp::Store, gfx::ResourceAccessType::Present);
	m_renderData->backbuffer = device->createBackbuffer(m_renderData->renderPass);
}

};

#endif