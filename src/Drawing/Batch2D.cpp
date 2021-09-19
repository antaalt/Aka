#include <Aka/Drawing/Batch2D.h>

#include <algorithm>

namespace aka {

#if defined(AKA_USE_OPENGL)
// Move somewhere else
static const char* vertShader =
"#version 330\n"
"layout (location = 0) in vec2 a_position;\n"
"layout (location = 1) in vec2 a_uv;\n"
"layout (location = 2) in vec4 a_color;\n"
"layout (std140) uniform ModelUniformBuffer { mat4 u_mvp; };\n"
"out vec2 v_uv;\n"
"out vec4 v_color;\n"
"void main(void) {\n"
"	gl_Position = u_mvp * vec4(a_position.xy, 0.0, 1.0);\n"
"	v_uv = a_uv;\n"
"	v_color = a_color;\n"
"}"
"";
static const char* fragShader =
"#version 330\n"
"in vec2 v_uv;\n"
"in vec4 v_color;\n"
"uniform sampler2D u_texture;\n"
"out vec4 o_color;\n"
"void main(void) {\n"
"	o_color = v_color * texture(u_texture, v_uv);\n"
"}"
"";
#elif defined(AKA_USE_D3D11)
static const char* vertShader = ""
"cbuffer ModelUniformBuffer : register(b0)\n"
"{\n"
"	float4x4 u_mvp;\n"
"}\n"
"struct vs_in\n"
"{\n"
"	float2 position : POSITION;\n"
"	float2 texcoord : TEXCOORD;\n"
"	float4 color : COLOR;\n"
"};\n"
"struct vs_out\n"
"{\n"
"	float4 position : SV_POSITION;\n"
"	float2 texcoord : TEXCOORD;\n"
"	float4 color : COLOR;\n"
"};\n"
"vs_out main(vs_in input)\n"
"{\n"
"	vs_out output;\n"
"	output.position = mul(u_mvp, float4(input.position, 0.0f, 1.0f));\n"
"	output.texcoord = input.texcoord;\n"
"	output.color = input.color;\n"
"	return output;\n"
"}\n";
static const char* fragShader = ""
"struct vs_out\n"
"{\n"
"	float4 position : SV_POSITION;\n"
"	float2 texcoord : TEXCOORD;\n"
"	float4 color : COLOR;\n"
"};\n"

"Texture2D    u_texture : register(t0);\n"
"SamplerState u_sampler : register(s0);\n"

"float4 main(vs_out input) : SV_TARGET\n"
"{\n"
"	return input.color * u_texture.Sample(u_sampler, input.texcoord);\n"
"}\n";
#endif

void Batch2D::draw(const mat3f& transform, const Line& line)
{
	DrawBatch& currentBatch = get(PrimitiveType::Lines, nullptr, line.layer);
	uint32_t offset = static_cast<uint32_t>(m_vertices.size());
	m_indices.push_back(offset + 0);
	m_indices.push_back(offset + 1);
	for (const Vertex& vert : line.vertices)
		m_vertices.push_back(vert);
	for (size_t iVert = offset; iVert < offset + 2; iVert++)
		m_vertices[iVert].position = transform.multiplyPoint(m_vertices[iVert].position);
#if defined(AKA_ORIGIN_TOP_LEFT)
	for (size_t iVert = offset; iVert < offset + 2; iVert++)
		m_vertices[iVert].uv.v = 1.f - m_vertices[iVert].uv.v;
#endif
	currentBatch.indexCount += 2;
}

void Batch2D::draw(const mat3f& transform, const Triangle& tri)
{
	DrawBatch& currentBatch = get(PrimitiveType::Triangles, tri.texture, tri.layer);
	uint32_t offset = static_cast<uint32_t>(m_vertices.size());
	m_indices.push_back(offset + 0);
	m_indices.push_back(offset + 1);
	m_indices.push_back(offset + 2);
	for (const Vertex& vert : tri.vertices)
		m_vertices.push_back(vert);
	for (size_t iVert = offset; iVert < offset + 3; iVert++)
		m_vertices[iVert].position = transform.multiplyPoint(m_vertices[iVert].position);
#if defined(AKA_ORIGIN_TOP_LEFT)
	for (size_t iVert = offset; iVert < offset + 3; iVert++)
		m_vertices[iVert].uv.v = 1.f - m_vertices[iVert].uv.v;
#endif
	currentBatch.indexCount += 3;
}

void Batch2D::draw(const mat3f& transform, const Quad& quad)
{
	// 	2___3
	// 	|   |
	//	0___1
	DrawBatch& currentBatch = get(PrimitiveType::Triangles, quad.texture, quad.layer);
	uint32_t offset = static_cast<uint32_t>(m_vertices.size());
	m_indices.push_back(offset + 0);
	m_indices.push_back(offset + 1);
	m_indices.push_back(offset + 2);
	m_indices.push_back(offset + 2);
	m_indices.push_back(offset + 1);
	m_indices.push_back(offset + 3);
	for (const Vertex& vert : quad.vertices)
		m_vertices.push_back(vert);
	for (size_t iVert = offset; iVert < offset + 4; iVert++)
		m_vertices[iVert].position = transform.multiplyPoint(m_vertices[iVert].position);
#if defined(AKA_ORIGIN_TOP_LEFT)
	for (size_t iVert = offset; iVert < offset + 4; iVert++)
		m_vertices[iVert].uv.v = 1.f - m_vertices[iVert].uv.v;
#endif
	currentBatch.indexCount += 6;
}

void Batch2D::draw(const mat3f& transform, const Poly& poly)
{
	DrawBatch& currentBatch = get(poly.primitive, poly.texture, poly.layer);
	size_t offset = m_vertices.size();

	for (size_t i = 0; i < poly.vertices.size(); i++)
		m_indices.push_back(static_cast<uint32_t>(offset + i));
	for (const Vertex& vertex : poly.vertices)
		m_vertices.push_back(vertex);
	for (size_t iVert = offset; iVert < offset + poly.vertices.size(); iVert++)
		m_vertices[iVert].position = transform.multiplyPoint(m_vertices[iVert].position);
#if defined(AKA_ORIGIN_TOP_LEFT)
	for (size_t iVert = offset; iVert < offset + poly.vertices.size(); iVert++)
		m_vertices[iVert].uv.v = 1.f - m_vertices[iVert].uv.v;
#endif
	currentBatch.indexCount += static_cast<uint32_t>(poly.vertices.size());
}

Batch2D::Batch2D() :
	m_program(nullptr),
	m_mesh(nullptr),
	m_defaultTexture(nullptr)
{
}

Batch2D::~Batch2D()
{
	destroy();
}

void Batch2D::initialize()
{
	std::vector<VertexAttribute> att {
		VertexAttribute{ VertexSemantic::Position, VertexFormat::Float, VertexType::Vec2 },
		VertexAttribute{ VertexSemantic::TexCoord0, VertexFormat::Float, VertexType::Vec2 },
		VertexAttribute{ VertexSemantic::Color0, VertexFormat::Float, VertexType::Vec4 }
	};
	Shader::Ptr vert = Shader::compile(vertShader, ShaderType::Vertex);
	Shader::Ptr frag = Shader::compile(fragShader, ShaderType::Fragment);
	m_program = Program::createVertexProgram(
		Shader::compile(vertShader, ShaderType::Vertex),
		Shader::compile(fragShader, ShaderType::Fragment),
		att.data(), att.size()
	);
	m_material = Material::create(m_program);

	m_uniformBuffer = Buffer::create(BufferType::Uniform, sizeof(mat4f), BufferUsage::Default, BufferCPUAccess::None);
	m_material->set("ModelUniformBuffer", m_uniformBuffer);

	m_mesh = Mesh::create();
	m_maxVertices = (1 << 9);
	m_maxIndices = (1 << 8);
	m_vertexBuffer = Buffer::create(BufferType::Vertex, m_maxVertices * sizeof(Vertex), BufferUsage::Dynamic, BufferCPUAccess::Write);
	m_indexBuffer = Buffer::create(BufferType::Index, m_maxIndices * sizeof(uint32_t), BufferUsage::Dynamic, BufferCPUAccess::Write);

	uint8_t data[4] = { 255, 255, 255, 255 };
	m_defaultTexture = Texture2D::create(1, 1, TextureFormat::RGBA8, TextureFlag::ShaderResource, data);

	m_pass = {};
	m_pass.clear = Clear::none;
	m_pass.blend = Blending::premultiplied;
	m_pass.cull = Culling::none;
	m_pass.depth = Depth::none;
	m_pass.stencil = Stencil::none;
	m_pass.submesh.mesh = m_mesh;
	m_pass.material = m_material;
}

void Batch2D::destroy()
{
}

Batch2D::DrawBatch& Batch2D::create(PrimitiveType type, Texture::Ptr texture, int32_t layer)
{
	size_t offset = (m_batches.size() == 0) ? 0 : m_batches.back().indexOffset + m_batches.back().indexCount;
	m_batches.emplace_back();
	m_batches.back().texture = texture;
	m_batches.back().indexOffset = static_cast<uint32_t>(offset);
	m_batches.back().indexCount = 0;
	m_batches.back().layer = layer;
	m_batches.back().primitive = type;
	return m_batches.back();
}

Batch2D::DrawBatch& Batch2D::get(PrimitiveType type, Texture::Ptr texture, int32_t layer)
{
	if (m_batches.size() == 0)
		return create(type, texture, layer);
	if (m_batches.back().indexCount == 0)
	{
		m_batches.back().texture = texture;
		m_batches.back().layer = layer;
		m_batches.back().primitive = type;
		return m_batches.back();
	}
	if (m_batches.back().layer != layer || m_batches.back().texture != texture || m_batches.back().primitive != type)
		return create(type, texture, layer);
	return m_batches.back();
}

void Batch2D::clear()
{
	m_batches.clear();
	m_vertices.clear();
	m_indices.clear();
}

size_t Batch2D::batchCount() const
{
	return m_batches.size();
}

size_t Batch2D::verticesCount() const
{
	return m_vertices.size();
}

size_t Batch2D::indicesCount() const
{
	return m_indices.size();
}

void Batch2D::render()
{
	render(GraphicBackend::backbuffer());
}

void Batch2D::render(Framebuffer::Ptr framebuffer)
{
	render(framebuffer, mat4f::identity());
}

void Batch2D::render(Framebuffer::Ptr framebuffer, const mat4f& view)
{
	render(framebuffer, view, mat4f::orthographic(0.f, (float)framebuffer->height(), 0.f, (float)framebuffer->width()));
}

void Batch2D::render(Framebuffer::Ptr framebuffer, const mat4f& view, const mat4f& projection)
{
	if (m_program == nullptr)
		initialize();
	
	{
		if (m_vertices.size() > m_maxVertices)
		{
			while (m_vertices.size() > m_maxVertices)
				m_maxVertices *= 2;
			m_vertexBuffer->reallocate(m_maxVertices * sizeof(Vertex));
		}
		void* data = m_vertexBuffer->map(BufferMap::WriteDiscard);
		memcpy(data, m_vertices.data(), m_vertices.size() * sizeof(Vertex));
		m_vertexBuffer->unmap();

		if (m_indices.size() > m_maxIndices)
		{
			while (m_indices.size() > m_maxIndices)
				m_maxIndices *= 2;
			m_indexBuffer->reallocate(m_maxIndices * sizeof(uint32_t));
		}
		data = m_indexBuffer->map(BufferMap::WriteDiscard);
		memcpy(data, m_indices.data(), m_indices.size() * sizeof(uint32_t));
		m_indexBuffer->unmap();

		// Update mesh data
		std::vector<VertexAccessor> vertexInfo{ {
			VertexAccessor{
				VertexAttribute{ VertexSemantic::Position, VertexFormat::Float, VertexType::Vec2 },
				VertexBufferView { m_vertexBuffer, 0, static_cast<uint32_t>(m_vertices.size() * sizeof(Vertex)), sizeof(Vertex) },
				offsetof(Vertex, position), static_cast<uint32_t>(m_vertices.size())
			},
			VertexAccessor{
				VertexAttribute{ VertexSemantic::TexCoord0, VertexFormat::Float, VertexType::Vec2 },
				VertexBufferView { m_vertexBuffer, 0, static_cast<uint32_t>(m_vertices.size() * sizeof(Vertex)), sizeof(Vertex) },
				offsetof(Vertex, uv), static_cast<uint32_t>(m_vertices.size())
			},
			VertexAccessor{
				VertexAttribute{ VertexSemantic::Color0, VertexFormat::Float, VertexType::Vec4 },
				VertexBufferView { m_vertexBuffer, 0, static_cast<uint32_t>(m_vertices.size() * sizeof(Vertex)) },
				offsetof(Vertex, color), static_cast<uint32_t>(m_vertices.size())
			},
		} };

		IndexAccessor indexInfo{
			IndexBufferView { m_indexBuffer, 0, static_cast<uint32_t>(m_indices.size() * sizeof(uint32_t)) },
			IndexFormat::UnsignedInt
		};
		m_mesh->upload(vertexInfo.data(), vertexInfo.size(), indexInfo);
	}

	{
		// Prepare renderPass
		m_pass.framebuffer = framebuffer;
		m_pass.viewport = aka::Rect{ 0, 0, framebuffer->width(), framebuffer->height() };
		m_pass.scissor = aka::Rect{ 0 };
		mat4f mvp = projection * view;
		m_uniformBuffer->upload(&mvp);
	}

	// Sort batches by layer before rendering
	// TODO use depth instead
	std::sort(m_batches.begin(), m_batches.end(), [](const DrawBatch &lhs, const DrawBatch &rhs) {
		return lhs.layer < rhs.layer;
	});

	// Draw the batches
	for (const DrawBatch &batch : m_batches)
	{
		// TODO draw instanced & pass model matrix, textures & offset / count.
		m_material->set("u_texture", TextureSampler::nearest); // TODO do not force nearest
		m_material->set("u_texture", batch.texture ? batch.texture : m_defaultTexture);
		m_pass.submesh.count = batch.indexCount;
		m_pass.submesh.offset = batch.indexOffset;
		m_pass.submesh.type = batch.primitive;
		m_pass.execute();
	}
}

};