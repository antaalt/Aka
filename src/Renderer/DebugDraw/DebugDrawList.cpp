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
	reserveBlock(8 * 3);
	const DebugVertex s_planeVertices[8] = {
		DebugVertex{transform.multiplyPoint(point3f(-1, -1, -1)), color}, // 0
		DebugVertex{transform.multiplyPoint(point3f( 1, -1, -1)), color}, // 1
		DebugVertex{transform.multiplyPoint(point3f(-1,  1, -1)), color}, // 2
		DebugVertex{transform.multiplyPoint(point3f(-1, -1, 1)), color}, // 3
		DebugVertex{transform.multiplyPoint(point3f( 1,  1, -1)), color}, // 4
		DebugVertex{transform.multiplyPoint(point3f(-1,  1,  1)), color}, // 5
		DebugVertex{transform.multiplyPoint(point3f( 1, -1,  1)), color}, // 6
		DebugVertex{transform.multiplyPoint(point3f( 1,  1,  1)), color}, // 7
	};
	// Face 1
	m_vertices.append(s_planeVertices[0]);
	m_vertices.append(s_planeVertices[1]);
	m_vertices.append(s_planeVertices[3]);
	m_vertices.append(s_planeVertices[6]);
	m_vertices.append(s_planeVertices[0]);
	m_vertices.append(s_planeVertices[3]);
	m_vertices.append(s_planeVertices[1]);
	m_vertices.append(s_planeVertices[6]);

	// Face 2
	m_vertices.append(s_planeVertices[2]);
	m_vertices.append(s_planeVertices[4]);
	m_vertices.append(s_planeVertices[5]);
	m_vertices.append(s_planeVertices[7]);
	m_vertices.append(s_planeVertices[2]);
	m_vertices.append(s_planeVertices[5]);
	m_vertices.append(s_planeVertices[4]);
	m_vertices.append(s_planeVertices[7]);

	// Face connect
	m_vertices.append(s_planeVertices[0]);
	m_vertices.append(s_planeVertices[2]);
	m_vertices.append(s_planeVertices[3]);
	m_vertices.append(s_planeVertices[5]);
	m_vertices.append(s_planeVertices[1]);
	m_vertices.append(s_planeVertices[4]);
	m_vertices.append(s_planeVertices[6]);
	m_vertices.append(s_planeVertices[7]);
}
void DebugDrawList::draw3DSphere(const mat4f& transform, const color4f& color)
{
	// http://www.songho.ca/opengl/gl_sphere.html
	const uint32_t segmentCount = 16;
	const uint32_t ringCount = 16;

	reserveBlock((segmentCount - 1) * ringCount * 6);

	anglef sectorStep = 2.f * pi<float> / (float)ringCount;
	anglef stackStep = pi<float> / (float)segmentCount;
	anglef ringAngle, segmentAngle;

	Vector<DebugVertex> vertices;
	vertices.reserve((segmentCount + 1) * (ringCount + 1));
	for (uint32_t i = 0; i <= segmentCount; ++i)
	{
		segmentAngle = pi<float> / 2.f - (float)i * stackStep; // starting from pi/2 to -pi/2
		float xy = cos(segmentAngle); // r * cos(u)
		float z = sin(segmentAngle); // r * sin(u)

		// add (ringCount+1) vertices per segment
		// the first and last vertices have same position and normal, but different uv
		for (uint32_t j = 0; j <= ringCount; ++j)
		{
			DebugVertex& v = vertices.emplace();
			ringAngle = (float)j * sectorStep; // starting from 0 to 2pi
			v.point[0] = xy * cos(ringAngle); // r * cos(u) * cos(v)
			v.point[1] = xy * sin(ringAngle); // r * cos(u) * sin(v)
			v.point[2] = z;
			v.point = transform.multiplyPoint(v.point);
			v.color = color;
		}
	}
	for (uint32_t i = 0; i < segmentCount; ++i)
	{
		uint32_t k1 = i * (ringCount + 1);     // beginning of current stack
		uint32_t k2 = k1 + ringCount + 1;      // beginning of next stack

		for (uint32_t j = 0; j < ringCount; ++j, ++k1, ++k2)
		{
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if (i != 0)
			{
				m_vertices.append(vertices[k1]);
				m_vertices.append(vertices[k2]);
				m_vertices.append(vertices[k1 + 1]);
			}
			// k1+1 => k2 => k2+1
			if (i != (segmentCount - 1))
			{
				m_vertices.append(vertices[k1 + 1]);
				m_vertices.append(vertices[k2]);
				m_vertices.append(vertices[k2 + 1]);
			}
		}
	}
}
void DebugDrawList::draw3DPlane(const mat4f& transform, const color4f& color)
{
	reserveBlock(8);
	// Vertices
	const DebugVertex s_planeVertices[4] = {
		DebugVertex{transform.multiplyPoint(point3f(-1.0f, -1.0f, 0.0f)), color},
		DebugVertex{transform.multiplyPoint(point3f(-1.0f,  1.0f, 0.0f)), color},
		DebugVertex{transform.multiplyPoint(point3f(1.0f,   1.0f, 0.0f)), color},
		DebugVertex{transform.multiplyPoint(point3f(1.0f,  -1.0f, 0.0f)), color},
	};
	m_vertices.append(s_planeVertices[0]);
	m_vertices.append(s_planeVertices[1]);
	m_vertices.append(s_planeVertices[1]);
	m_vertices.append(s_planeVertices[2]);
	m_vertices.append(s_planeVertices[2]);
	m_vertices.append(s_planeVertices[3]);
	m_vertices.append(s_planeVertices[3]);
	m_vertices.append(s_planeVertices[0]);
}
void DebugDrawList::draw3DLine(const point3f* positions, size_t count, const color4f& color)
{
	// Merge
	for (size_t i = 0; i < count; i++)
	{
		m_vertices.append(DebugVertex{ positions[i], color });
	}
}
static const size_t s_blockSize = 10'000;
void DebugDrawList::reserveBlock(size_t _count)
{
	// Reserve a new block if not enough size to avoid allocation overhead.
	if (m_vertices.size() + _count > m_vertices.capacity())
	{
		m_vertices.reserve(m_vertices.capacity() + s_blockSize);
	}
}

void DebugDrawList::create(gfx::GraphicDevice* _device, uint32_t width, uint32_t height)
{
	const ShaderKey ShaderVertex = ShaderKey::generate(AssetPath("shaders/debugdraw/debug.vert", AssetPathType::Common), ShaderType::Vertex);
	const ShaderKey ShaderFragment = ShaderKey::generate(AssetPath("shaders/debugdraw/debug.frag", AssetPathType::Common), ShaderType::Fragment);

	const ProgramKey ProgramGraphic = ProgramKey().add(ShaderVertex).add(ShaderFragment);
	ShaderRegistry* program = Application::app()->program();
	program->add(ProgramGraphic, _device);
	gfx::ProgramHandle m_program = program->get(ProgramGraphic);


	gfx::ShaderConstant constant;
	constant.offset = 0;
	constant.size = sizeof(mat4f);
	constant.shader = gfx::ShaderMask::Vertex;
	gfx::ShaderPipelineLayout layout{};
	layout.addConstant(constant);

	gfx::RenderPassState state{};
	state.addColor(gfx::TextureFormat::Swapchain, gfx::AttachmentLoadOp::Load, gfx::AttachmentStoreOp::Store, gfx::ResourceAccessType::Present, gfx::ResourceAccessType::Present);
	state.setDepth(gfx::TextureFormat::Depth24Stencil8, gfx::AttachmentLoadOp::Clear, gfx::AttachmentStoreOp::Store, gfx::ResourceAccessType::Attachment, gfx::ResourceAccessType::Attachment);

	// TODO: share depth & render pass between renderer
	m_depth = _device->createTexture("StaticDepth", width, height, 1, gfx::TextureType::Texture2D, 1, 1, gfx::TextureFormat::Depth24Stencil8, gfx::TextureUsage::RenderTarget);
	m_backbufferRenderPass = _device->createRenderPass("BackbufferPassHandle", state);
	gfx::Attachment depthAttachment;
	depthAttachment.texture = m_depth;
	depthAttachment.flag = gfx::AttachmentFlag::BackbufferAutoResize;
	m_backbuffer = _device->createBackbuffer("Backbuffer", m_backbufferRenderPass, nullptr, 0, &depthAttachment);

	m_pipeline = _device->createGraphicPipeline(
		"DebugDraw",
		m_program,
		gfx::PrimitiveType::Lines,
		layout,
		_device->get(m_backbufferRenderPass)->state,
		gfx::VertexState{}.add(DebugVertex::getVertexLayout()),
		gfx::ViewportStateBackbuffer,
		gfx::DepthStateLessEqual,
		gfx::StencilStateDisabled,
		gfx::CullStateDefault,
		gfx::BlendStateDefault,
		gfx::FillStateLine
	);
	for (uint32_t i = 0; i < gfx::MaxFrameInFlight; i++)
	{
		m_vertices.reserve(s_blockSize);
		m_vertexBufferSize[i] = s_blockSize;
		m_vertexBuffer[i] = _device->createBuffer("DebugDrawVertexBuffer", gfx::BufferType::Vertex, (uint32_t)(sizeof(DebugVertex) * s_blockSize), gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);
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
		if (m_vertexBufferSize[frameIndex.value()] < m_vertices.size())
		{
			_device->destroy(m_vertexBuffer[frameIndex.value()]);
			m_vertexBufferSize[frameIndex.value()] = m_vertices.size();
			m_vertexBuffer[frameIndex.value()] = _device->createBuffer("DebugDrawVertexBuffer", gfx::BufferType::Vertex, (uint32_t)(sizeof(DebugVertex) * m_vertices.size()), gfx::BufferUsage::Default, gfx::BufferCPUAccess::None);
		}
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
		gfx::ScopedCmdMarker marker(cmd, "DebugDrawList", 0.f, 0.f, 0.5f);
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