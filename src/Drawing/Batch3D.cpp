#include <Aka/Drawing/Batch3D.h>

#include <algorithm>

namespace aka {

#if defined(AKA_USE_OPENGL)
// Move somewhere else
static const char* vertShader =
"#version 330\n"
"layout (location = 0) in vec3 a_position;\n"
"layout (location = 1) in vec3 a_normal;\n"
"layout (location = 2) in vec2 a_uv;\n"
"layout (location = 3) in vec4 a_color;\n"
"layout (std140) uniform ModelUniformBuffer { mat4 u_mvp; };\n"
"out vec3 v_position; \n"
"out vec2 v_uv; \n"
"out vec4 v_color; \n"
"void main(void) {\n"
"	gl_Position = u_mvp * vec4(a_position, 1.0);\n"
"	v_position = gl_Position.xyz;\n"
"	v_uv = a_uv;\n"
"	v_color = a_color;\n"
"}"
"";
static const char* fragShader =
"#version 330\n"
"in vec3 v_position;\n"
"in vec2 v_uv;\n"
"in vec4 v_color;\n"
"uniform sampler2D u_texture;\n"
"out vec4 o_color;\n"
"void main(void) {\n"
"	o_color = v_color * texture(u_texture, v_uv);\n"
"}"
"";
#elif defined(AKA_USE_D3D11)
static const char* shader = ""
"cbuffer ModelUniformBuffer : register(b0)\n"
"{\n"
"	row_major float4x4 u_mvp;\n"
"}\n"

"struct vs_in\n"
"{\n"
"	float3 position : POS;\n"
"	float3 normal : NORM;\n"
"	float2 texcoord : TEX;\n"
"	float4 color : COL;\n"
"};\n"

"struct vs_out\n"
"{\n"
"	float4 position : SV_POSITION;\n"
"	float2 texcoord : TEX;\n"
"	float4 color : COL;\n"
"};\n"

"Texture2D    u_texture : register(t0);\n"
"SamplerState u_sampler : register(s0);\n"

"vs_out vs_main(vs_in input)\n"
"{\n"
"	vs_out output;\n"

"	output.position = mul(float4(input.position, 1.0f), u_mvp);\n"
"	output.texcoord = input.texcoord;\n"
"	output.color = input.color;\n"

"	return output;\n"
"}\n"

"float4 ps_main(vs_out input) : SV_TARGET\n"
"{\n"
"	return input.color * u_texture.Sample(u_sampler, input.texcoord);\n"
"}\n";
static const char* vertShader = shader;
static const char* fragShader = shader;
#endif

void Batch3D::draw(const mat4f& transform, const Triangle& triangle)
{
	DrawBatch& currentBatch = get(PrimitiveType::Triangles, triangle.texture);

	size_t offset = static_cast<uint32_t>(m_vertices.size());
	m_indices.push_back(static_cast<uint32_t>(offset + 0));
	m_indices.push_back(static_cast<uint32_t>(offset + 1));
	m_indices.push_back(static_cast<uint32_t>(offset + 2));
	for (const Vertex& vertex : triangle.vertices)
		m_vertices.push_back(vertex);
	for (size_t iVert = offset; iVert < offset + 3; iVert++)
		m_vertices[iVert].position = transform.multiplyPoint(m_vertices[iVert].position);
	currentBatch.indexCount += 3;
}

void Batch3D::draw(const mat4f& transform, const Line& line)
{
	DrawBatch& currentBatch = get(PrimitiveType::Lines, line.texture);

	size_t offset = static_cast<uint32_t>(m_vertices.size());
	m_indices.push_back(static_cast<uint32_t>(offset + 0));
	m_indices.push_back(static_cast<uint32_t>(offset + 1));
	for (const Vertex& vertex : line.vertices)
		m_vertices.push_back(vertex);
	for (size_t iVert = offset; iVert < offset + 2; iVert++)
		m_vertices[iVert].position = transform.multiplyPoint(m_vertices[iVert].position);
	currentBatch.indexCount += 2;
}

void Batch3D::draw(const mat4f& transform, const Quad& quad)
{
	DrawBatch& currentBatch = get(PrimitiveType::Triangles, quad.texture);

	size_t offset = m_vertices.size();
	m_indices.push_back(static_cast<uint32_t>(offset + 0));
	m_indices.push_back(static_cast<uint32_t>(offset + 1));
	m_indices.push_back(static_cast<uint32_t>(offset + 2));
	m_indices.push_back(static_cast<uint32_t>(offset + 2));
	m_indices.push_back(static_cast<uint32_t>(offset + 1));
	m_indices.push_back(static_cast<uint32_t>(offset + 3));
	for (const Vertex& vertex : quad.vertices)
		m_vertices.push_back(vertex);
	for (size_t iVert = offset; iVert < offset + 6; iVert++)
		m_vertices[iVert].position = transform.multiplyPoint(m_vertices[iVert].position);
	currentBatch.indexCount += 6;
}

void Batch3D::draw(const mat4f& transform, const Poly& poly)
{
	DrawBatch& currentBatch = get(poly.primitive, poly.texture);

	size_t offset = m_vertices.size();
	for (size_t i = 0; i < poly.vertices.size(); i++)
		m_indices.push_back(static_cast<uint32_t>(offset + i));
	for (const Vertex& vertex : poly.vertices)
		m_vertices.push_back(vertex);
	for (size_t iVert = offset; iVert < offset + poly.vertices.size(); iVert++)
		m_vertices[iVert].position = transform.multiplyPoint(m_vertices[iVert].position);
	currentBatch.indexCount += static_cast<uint32_t>(poly.vertices.size());
}

Batch3D::Batch3D() :
	m_shader(nullptr),
	m_mesh(nullptr),
	m_defaultTexture(nullptr)
{
}

Batch3D::~Batch3D()
{
	destroy();
}

void Batch3D::initialize()
{
	std::vector<VertexAttribute> att{
		VertexAttribute{ VertexSemantic::Position, VertexFormat::Float, VertexType::Vec3 },
		VertexAttribute{ VertexSemantic::Normal, VertexFormat::Float, VertexType::Vec3 },
		VertexAttribute{ VertexSemantic::TexCoord0, VertexFormat::Float, VertexType::Vec2 },
		VertexAttribute{ VertexSemantic::Color0, VertexFormat::Float, VertexType::Vec4 }
	};
	ShaderHandle vert = Shader::compile(vertShader, ShaderType::Vertex);
	ShaderHandle frag = Shader::compile(fragShader, ShaderType::Fragment);
	m_shader = Shader::createVertexProgram(
		vert,
		frag,
		att.data(), att.size()
	);
	m_material = ShaderMaterial::create(m_shader);
	Shader::destroy(vert);
	Shader::destroy(frag);

	m_uniformBuffer = Buffer::create(BufferType::Uniform, sizeof(mat4f), BufferUsage::Default, BufferCPUAccess::None);
	m_material->set("ModelUniformBuffer", m_uniformBuffer);

	m_mesh = Mesh::create();
	m_maxVertices = (1 << 9);
	m_maxIndices = (1 << 8);
	m_vertexBuffer = Buffer::create(BufferType::Vertex, m_maxVertices * sizeof(Vertex), BufferUsage::Dynamic, BufferCPUAccess::Write);
	m_indexBuffer = Buffer::create(BufferType::Index, m_maxIndices * sizeof(uint32_t), BufferUsage::Dynamic, BufferCPUAccess::Write);

	uint8_t data[4] = { 255, 255, 255, 255 };
	m_defaultTexture = Texture::create2D(1, 1, TextureFormat::RGBA8, TextureFlag::None, data);

	m_pass = {};
	m_pass.clear = Clear::none;
	m_pass.blend = Blending::premultiplied;
	m_pass.cull = Culling::none;
	m_pass.depth = Depth::none;
	m_pass.stencil = Stencil::none;
	m_pass.submesh.mesh = m_mesh;
	m_pass.submesh.type = PrimitiveType::Triangles;
	m_pass.material = m_material;
}

void Batch3D::destroy()
{
}

bool Batch3D::DrawBatch::operator==(const DrawBatch& batch) const
{
	return texture == batch.texture && primitive == batch.primitive;
}

Batch3D::DrawBatch& Batch3D::create(PrimitiveType primitive, Texture::Ptr texture)
{
	size_t offset = (m_batches.size() == 0) ? 0 : m_batches.back().indexOffset + m_batches.back().indexCount;
	m_batches.emplace_back();
	m_batches.back().texture = texture;
	m_batches.back().indexOffset = static_cast<uint32_t>(offset);
	m_batches.back().indexCount = 0;
	m_batches.back().primitive = primitive;
	return m_batches.back();
}

Batch3D::DrawBatch& Batch3D::get(PrimitiveType primitive, Texture::Ptr texture)
{
	if (m_batches.size() == 0)
		return create(primitive, texture);
	if (m_batches.back().primitive != primitive || m_batches.back().texture != texture)
		return create(primitive, texture);
	return m_batches.back();
}

void Batch3D::clear()
{
	m_indices.clear();
	m_vertices.clear();
	m_batches.clear();
}

void Batch3D::render(Framebuffer::Ptr framebuffer, const mat4f& view, const mat4f& projection)
{
	if (m_shader == nullptr)
		initialize();

	{
		if (m_vertices.size() > m_maxVertices)
		{
			while(m_vertices.size() > m_maxVertices)
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
				VertexAttribute{ VertexSemantic::Position, VertexFormat::Float, VertexType::Vec3 },
				VertexBufferView { m_vertexBuffer, 0, static_cast<uint32_t>(m_vertices.size() * sizeof(Vertex)), sizeof(Vertex) },
				offsetof(Vertex, position), static_cast<uint32_t>(m_vertices.size())
			},
			VertexAccessor{
				VertexAttribute{ VertexSemantic::Normal, VertexFormat::Float, VertexType::Vec3 },
				VertexBufferView { m_vertexBuffer, 0, static_cast<uint32_t>(m_vertices.size() * sizeof(Vertex)), sizeof(Vertex) },
				offsetof(Vertex, normal), static_cast<uint32_t>(m_vertices.size())
			},
			VertexAccessor{
				VertexAttribute{ VertexSemantic::TexCoord0, VertexFormat::Float, VertexType::Vec2 },
				VertexBufferView { m_vertexBuffer, 0, static_cast<uint32_t>(m_vertices.size() * sizeof(Vertex)), sizeof(Vertex) },
				offsetof(Vertex, uv), static_cast<uint32_t>(m_vertices.size())
			},
			VertexAccessor{
				VertexAttribute{ VertexSemantic::Color0, VertexFormat::Float, VertexType::Vec4 },
				VertexBufferView { m_vertexBuffer, 0, static_cast<uint32_t>(m_vertices.size() * sizeof(Vertex)), sizeof(Vertex) },
				offsetof(Vertex, color), static_cast<uint32_t>(m_vertices.size())
			}
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

	// Draw the batches
	size_t iTransform = 0;
	for (const DrawBatch& batch : m_batches)
	{
		m_material->set("u_texture", TextureSampler::nearest); // TODO do not force nearest
		m_material->set("u_texture", batch.texture ? batch.texture : m_defaultTexture);
		m_pass.submesh.type = batch.primitive;
		m_pass.submesh.count = batch.indexCount;
		m_pass.submesh.offset = batch.indexOffset;
		m_pass.execute();
	}
}

};