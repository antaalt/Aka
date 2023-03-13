# Aka Engine

Small 3D game engine, mainly built for personal educational purpose, to be easy to use and to extend. It was at start a small engine for 2D game developement with OpenGL and it suddenly became a 3D engine with Vulkan...

Aka stand for red in japanese (赤) and there is no particular reason for this name, we just need one !

## Features
- Platform backend with GLFW 3.2
- Graphic backend with Vulkan / D3D12 (WIP)
- Audio backend with RtAudio (mp3 support)
- Event system for intersystem communication
- UTF8 support
- Cross platform (should be easily compiled)
- ECS system based on entt

## How to use
This basic example create a window and spin a cube in the middle of the screen.
```cpp
#include <Aka/Aka.h>

using namespace aka;

static float vertices[] = {
	-1.0f,  1.0f, -1.0f,	0.f, 1.f,
	-1.0f, -1.0f, -1.0f,	0.f, 0.f,
	 1.0f, -1.0f, -1.0f,	1.f, 0.f,
	 1.0f, -1.0f, -1.0f,	1.f, 0.f,
	 1.0f,  1.0f, -1.0f,	1.f, 1.f,
	-1.0f,  1.0f, -1.0f,	0.f, 1.f,

	-1.0f, -1.0f,  1.0f,	0.f, 1.f,
	-1.0f, -1.0f, -1.0f,	0.f, 0.f,
	-1.0f,  1.0f, -1.0f,	1.f, 0.f,
	-1.0f,  1.0f, -1.0f,	1.f, 0.f,
	-1.0f,  1.0f,  1.0f,	1.f, 1.f,
	-1.0f, -1.0f,  1.0f,	0.f, 1.f,

	 1.0f, -1.0f, -1.0f,	0.f, 0.f,
	 1.0f, -1.0f,  1.0f,	0.f, 1.f,
	 1.0f,  1.0f,  1.0f,	1.f, 1.f,
	 1.0f,  1.0f,  1.0f,	1.f, 1.f,
	 1.0f,  1.0f, -1.0f,	1.f, 0.f,
	 1.0f, -1.0f, -1.0f,	0.f, 0.f,

	-1.0f, -1.0f,  1.0f,	0.f, 0.f,
	-1.0f,  1.0f,  1.0f,	0.f, 1.f,
	 1.0f,  1.0f,  1.0f,	1.f, 1.f,
	 1.0f,  1.0f,  1.0f,	1.f, 1.f,
	 1.0f, -1.0f,  1.0f,	1.f, 0.f,
	-1.0f, -1.0f,  1.0f,	0.f, 0.f,

	-1.0f,  1.0f, -1.0f,	0.f, 0.f,
	 1.0f,  1.0f, -1.0f,	1.f, 0.f,
	 1.0f,  1.0f,  1.0f,	1.f, 1.f,
	 1.0f,  1.0f,  1.0f,	1.f, 1.f,
	-1.0f,  1.0f,  1.0f,	0.f, 1.f,
	-1.0f,  1.0f, -1.0f,	0.f, 0.f,

	-1.0f, -1.0f, -1.0f,	0.f, 0.f,
	-1.0f, -1.0f,  1.0f,	0.f, 1.f,
	 1.0f, -1.0f, -1.0f,	1.f, 0.f,
	 1.0f, -1.0f, -1.0f,	1.f, 0.f,
	-1.0f, -1.0f,  1.0f,	0.f, 1.f,
	 1.0f, -1.0f,  1.0f,	1.f, 1.f,
};

const char* vertex_shader = R"(
#version 450
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_uv;
layout (location = 0) out vec2 v_uv;
layout(binding = 0) uniform UniformBuffer {
	mat4 model;
	mat4 view;
	mat4 projection;
} ubo;
void main()
{
	v_uv = a_uv;
	gl_Position = ubo.projection * ubo.view * ubo.model * vec4(a_position, 1.0);
}
)";

const char* fragment_shader = R"(
#version 450
layout(location = 0) in vec2 v_uv;
layout(location = 0) out vec4 o_color;
void main()
{
	o_color = vec4(1, 0, 1, 1);
}
)";

struct Game :
	Application,
	EventListener<KeyboardKeyDownEvent>
{
	gfx::BufferHandle ubo = gfx::BufferHandle::null;
	gfx::BufferHandle vertexBuffer = gfx::BufferHandle::null;
	gfx::ShaderHandle vertex = gfx::ShaderHandle::null;
	gfx::ShaderHandle fragment = gfx::ShaderHandle::null;
	gfx::ProgramHandle program = gfx::ProgramHandle::null;
	gfx::GraphicPipelineHandle pipeline = gfx::GraphicPipelineHandle::null;
	gfx::DescriptorSetHandle set = gfx::DescriptorSetHandle::null;
	gfx::RenderPassHandle renderPass = gfx::RenderPassHandle::null;
	gfx::BackbufferHandle backbuffer = gfx::BackbufferHandle::null;
	anglef rotation = anglef::radian(0.f);
	struct UBO {
		mat4f model;
		mat4f view;
		mat4f projection;
	};
	void onCreate(int argc, char* argv[]) override {
		UBO uboData;
		uboData.model = mat4f::identity();
		uboData.view = mat4f::lookAtView(point3f(3), point3f(0));
		uboData.projection = mat4f::perspective(anglef::degree(60.f), width() / (float)height(), 0.1f, 100.f);
		ubo = graphic()->createBuffer("UBO", gfx::BufferType::Uniform, sizeof(UBO), gfx::BufferUsage::Default, gfx::BufferCPUAccess::None, &uboData);
		vertexBuffer = graphic()->createBuffer("Vertices", gfx::BufferType::Vertex, sizeof(vertices), gfx::BufferUsage::Default, gfx::BufferCPUAccess::None, vertices);
		// Shaders
		ShaderCompiler compiler;
		Blob vertexBlob = compiler.compile(ShaderKey::fromString(vertex_shader, ShaderType::Vertex));
		Blob fragmentBlob = compiler.compile(ShaderKey::fromString(fragment_shader, ShaderType::Fragment));
		vertex = graphic()->createShader(gfx::ShaderType::Vertex, vertexBlob.data(), vertexBlob.size());
		fragment = graphic()->createShader(gfx::ShaderType::Fragment, fragmentBlob.data(), fragmentBlob.size());
		gfx::ShaderBindingState state = gfx::ShaderBindingState().add(gfx::ShaderBindingType::UniformBuffer, gfx::ShaderMask::Vertex, 1);
		program = graphic()->createGraphicProgram("", vertex, fragment, gfx::ShaderHandle::null, &state, 1);
		// Descriptors
		set = graphic()->createDescriptorSet("DescriptorSet", state);
		graphic()->update(set, gfx::DescriptorSetData().addUniformBuffer(ubo));
		// Renderpass
		renderPass = graphic()->createBackbufferRenderPass();
		backbuffer = graphic()->createBackbuffer(renderPass);
		// Pipeline
		pipeline = graphic()->createGraphicPipeline(program,
			gfx::PrimitiveType::Triangles,
			graphic()->get(renderPass)->state,
			gfx::VertexBindingState().
				add(gfx::VertexSemantic::Position, gfx::VertexFormat::Float, gfx::VertexType::Vec3).
				add(gfx::VertexSemantic::TexCoord0, gfx::VertexFormat::Float, gfx::VertexType::Vec2),
			gfx::ViewportState().size(width(), height()),
			gfx::DepthStateLessEqual,
			gfx::StencilStateDefault,
			gfx::CullStateCCW,
			gfx::BlendStateDefault,
			gfx::FillStateFill
		);
	}
	void onDestroy() override {
		graphic()->destroy(ubo);
		graphic()->destroy(vertexBuffer);
		graphic()->destroy(vertex);
		graphic()->destroy(fragment);
		graphic()->destroy(program);
		graphic()->destroy(pipeline);
		graphic()->destroy(set);
		graphic()->destroy(renderPass);
	}
	void onUpdate(Time deltaTime) override {
		rotation += anglef::radian(deltaTime.seconds());
	}
	void onRender(gfx::Frame* frame) override {
		gfx::GraphicDevice* device = graphic();
		vec2f size = aka::vec2f(300.f);
		vec2f position = aka::vec2f(
			width() / 2.f - size.x / 2.f,
			height() / 2.f - size.y / 2.f
		);
		mat3f transform = aka::mat3f::identity();
		transform *= aka::mat3f::translate(position);
		transform *= aka::mat3f::translate(0.5f * size);
		transform *= aka::mat3f::rotate(rotation);
		transform *= aka::mat3f::translate(-0.5f * size);
		transform *= aka::mat3f::scale(size);

		UBO* data = static_cast<UBO*>(device->map(ubo, gfx::BufferMap::Write));
		data->model = mat4f::rotate(vec3f(0, 1, 0), rotation);// mat4f::from2D(transform);
		data->view = mat4f::lookAtView(point3f(3), point3f(0));
		data->projection = mat4f::perspective(anglef::degree(60.f), width() / (float)height(), 0.1f, 100.f);
		device->unmap(ubo);

		gfx::CommandList* cmd = frame->getMainCommandList();
		//cmd->bindIndexBuffer(indexBuffer, gfx::IndexFormat::UnsignedInt);
		cmd->bindVertexBuffer(vertexBuffer, 0);
		cmd->bindPipeline(pipeline);
		cmd->bindDescriptorSet(0, set);
		cmd->beginRenderPass(renderPass, device->get(backbuffer, frame), gfx::ClearState{ gfx::ClearMask::All, {1.f, 1.f, 1.f, 1.f}, 1.f, 1 });
		cmd->draw(36, 0, 1);
		cmd->endRenderPass();
	}
	void onReceive(const KeyboardKeyDownEvent& event) override {
		if (event.key == KeyboardKey::Escape)
			EventDispatcher<QuitEvent>::emit();
	}
};
int main()
{
	Game game;
	aka::Config cfg;
	cfg.graphic.api = aka::gfx::GraphicAPI::Vulkan;
	cfg.platform.width = 1280;
	cfg.platform.height = 720;
	cfg.platform.name = "Game";
	cfg.app = &game;
	aka::Application::run(cfg);
	return 0;
}
```

## Build
-   Run --recursive with git clone or git submodule init / update to get all dependencies.
-   Build using CMake. It has been tested on Windows (VS 2022) & Linux (GCC).

## RoadMap
-   Add DX12 support
-   Some multithreading
-   An editor
-   Should probably go to Rust instead ?
