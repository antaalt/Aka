#include <Aka/Renderer/DebugDraw/DebugDrawList.hpp>

#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Core/Application.h>

namespace aka {

void DebugDrawList::draw3DFrustum(const mat4f& projection, const color4f& color)
{
	draw3DCube(mat4f::inverse(projection), color);
}
void DebugDrawList::draw3DCube(const mat4f& transform, const color4f& color)
{
	// Face 1
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(-1, -1, -1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(1, -1, -1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(-1,  1, -1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(1,  1, -1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(-1, -1, -1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(-1,  1, -1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(1, -1, -1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(1,  1, -1)), color4f(color) });

	// Face 2
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(-1, -1,  1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(1, -1,  1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(-1,  1,  1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(1,  1,  1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(-1, -1,  1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(-1,  1,  1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(1, -1,  1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(1,  1,  1)), color4f(color) });

	// Face connect
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(-1, -1, -1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(-1, -1,  1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(-1,  1, -1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(-1,  1,  1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(1, -1, -1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(1, -1,  1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(1,  1, -1)), color4f(color) });
	m_vertices.append(DebugVertex{ transform.multiplyPoint(point3f(1,  1,  1)), color4f(color) });
}
void DebugDrawList::draw3DLine(const point3f* positions, size_t count, const color4f& color)
{
	// Merge
	for (size_t i = 0; i < count; i++)
	{
		m_vertices.append(DebugVertex{ positions[i], color4f(color) });
	}
}

static const char* s_debugVertexShader = R"(
	#version 450
	layout(location = 0) in vec3 a_position;
	layout(location = 1) in vec4 a_color;
	layout(location = 0) out vec3 v_position;
	layout(location = 1) out vec4 v_color;
	layout(push_constant) uniform constants {
		mat4 mvp;
	} u_push;
	void main(void) {
		v_position = a_position;
		gl_Position = u_push.mvp * vec4(v_position, 1.0);
		v_color = a_color;
	}
)";

static const char* s_debugFragmentShader = R"(
	#version 450
	layout (location = 0) in vec3 v_position;
	layout (location = 1) in vec4 v_color;
	layout (location = 0) out vec4 o_color;
	void main(void) {
		o_color = v_color;
	}
)";

void DebugDrawList::create(gfx::GraphicDevice* _device, uint32_t width, uint32_t height)
{
	const ShaderKey ShaderVertex = ShaderKey::fromString(s_debugVertexShader, ShaderType::Vertex);
	const ShaderKey ShaderFragment = ShaderKey::fromString(s_debugFragmentShader, ShaderType::Fragment);

	const ProgramKey ProgramGraphic = ProgramKey().add(ShaderVertex).add(ShaderFragment);
	ShaderRegistry* program = Application::app()->program();
	program->add(ProgramGraphic, _device);
	gfx::ProgramHandle m_program = program->get(ProgramGraphic);

	m_backbufferRenderPass = _device->createBackbufferRenderPass(gfx::AttachmentLoadOp::Load, gfx::AttachmentStoreOp::Store, gfx::ResourceAccessType::Present);
	m_backbuffer = _device->createBackbuffer(m_backbufferRenderPass);
	m_pipeline = _device->createGraphicPipeline(
		"DebugDraw",
		m_program,
		gfx::PrimitiveType::Lines,
		_device->get(m_backbufferRenderPass)->state,
		gfx::VertexState{}.add(DebugVertex::getVertexLayout()),
		gfx::ViewportState{}.size(width, height),
		gfx::DepthStateLessEqual,
		gfx::StencilStateDisabled,
		gfx::CullStateDefault,
		gfx::BlendStateDefault,
		gfx::FillStateLine
	);
	for (uint32_t i = 0; i < gfx::MaxFrameInFlight; i++)
	{
		m_vertexBuffer[i] = _device->createBuffer("DebugDrawVertexBuffer", gfx::BufferType::Vertex, sizeof(DebugVertex) * 10000, gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);
	}
}

void DebugDrawList::destroy(gfx::GraphicDevice* _device)
{
	for (uint32_t i = 0; i < gfx::MaxFrameInFlight; i++)
	{
		_device->destroy(m_vertexBuffer[i]);
	}
	_device->destroy(m_pipeline);
	_device->destroy(m_backbufferRenderPass);
	_device->destroy(m_backbuffer);
}
void DebugDrawList::resize(gfx::GraphicDevice* _device, uint32_t width, uint32_t height)
{
	destroy(_device);
	create(_device, width, height);
}
void DebugDrawList::prepare(gfx::FrameHandle frame, gfx::GraphicDevice* _device)
{
	if (m_vertices.size() > 0)
	{
		// TOOD use staging buffer instead
		gfx::FrameIndex frameIndex = _device->getFrameIndex(frame);
		_device->upload(m_vertexBuffer[frameIndex.value()], m_vertices.data(), 0, m_vertices.size() * sizeof(DebugVertex));
	}
}
void DebugDrawList::render(gfx::GraphicDevice* _device, gfx::FrameHandle frame, const mat4f& view, const mat4f& projection)
{
	// Should have some wrapper such as GraphicCommandList (which only has 
	// RenderPassCommandList 
	// Setup a render pass & pass command list with lessen right access so that it can draw within same render pass.

	if (m_vertices.size() > 0)
	{
		gfx::FrameIndex frameIndex = _device->getFrameIndex(frame);
		gfx::CommandList* cmd = _device->getGraphicCommandList(frame);
		cmd->beginRenderPass(m_backbufferRenderPass, _device->get(m_backbuffer, frame));
		cmd->bindVertexBuffer(0, m_vertexBuffer[frameIndex.value()]);
		cmd->bindPipeline(m_pipeline);
		mat4f mvp = projection * view;
		cmd->push(0, sizeof(mat4f), &mvp, gfx::ShaderMask::Vertex);
		cmd->draw((uint32_t)m_vertices.size(), 0, 1, 0);
		cmd->endRenderPass();
	}
}
void DebugDrawList::clear()
{
	m_vertices.clear();
}


};