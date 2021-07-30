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
"uniform mat4 u_projection;\n"
"uniform mat4 u_view;\n"
"out vec2 v_uv;\n"
"out vec4 v_color;\n"
"void main(void) {\n"
"	gl_Position = u_projection * u_view * vec4(a_position.xy, 0.0, 1.0);\n"
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
static const char* shader = ""
"cbuffer constants : register(b0)\n"
"{\n"
"	row_major float4x4 u_view;\n"
"	row_major float4x4 u_projection;\n"
"}\n"

"struct vs_in\n"
"{\n"
"	float2 position : POS;\n"
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

"	output.position = mul(mul(float4(input.position, 0.0f, 1.0f), u_view), u_projection);\n"
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
#if defined(ORIGIN_TOP_LEFT)
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
#if defined(ORIGIN_TOP_LEFT)
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
#if defined(ORIGIN_TOP_LEFT)
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
#if defined(ORIGIN_TOP_LEFT)
	for (size_t iVert = offset; iVert < offset + poly.vertices.size(); iVert++)
		m_vertices[iVert].uv.v = 1.f - m_vertices[iVert].uv.v;
#endif
	currentBatch.indexCount += static_cast<uint32_t>(poly.vertices.size());
}

Batch2D::Batch2D() :
	m_shader(nullptr),
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
	m_shader = Shader::create(
		Shader::compile(vertShader, ShaderType::Vertex),
		Shader::compile(fragShader, ShaderType::Fragment),
		std::vector<Attributes>{ // HLSL only
			Attributes{ AttributeID(0), "POS" },
			Attributes{ AttributeID(0), "TEX" },
			Attributes{ AttributeID(0), "COL" }
		}
	);
	m_material = ShaderMaterial::create(m_shader);
	m_mesh = Mesh::create();
	uint8_t data[4] = { 255, 255, 255, 255 };
	m_defaultTexture = Texture::create2D(1, 1, TextureFormat::UnsignedByte, TextureComponent::RGBA, TextureFlag::None, Sampler::nearest(), data);

	m_pass = {};
	m_pass.clear = Clear{ ClearMask::None, color4f(1.f), 1.f, 0 };
	m_pass.blend = Blending::nonPremultiplied();
	m_pass.cull = Culling{ CullMode::None, CullOrder::CounterClockWise };
	m_pass.depth = Depth{ DepthCompare::None, true };
	m_pass.stencil = Stencil::none();
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
	if (m_shader == nullptr)
		initialize();
	
	{
		// Update mesh data
		m_mesh->indices(IndexFormat::UnsignedInt, m_indices.data(), m_indices.size());
		VertexData data;
		data.attributes.push_back(VertexData::Attribute{ 0, VertexFormat::Float, VertexType::Vec2 });
		data.attributes.push_back(VertexData::Attribute{ 1, VertexFormat::Float, VertexType::Vec2 });
		data.attributes.push_back(VertexData::Attribute{ 2, VertexFormat::Float, VertexType::Vec4 });
		m_mesh->vertices(data, m_vertices.data(), m_vertices.size());
	}

	{
		// Prepare renderPass
		m_pass.framebuffer = framebuffer;
		m_pass.viewport = aka::Rect{ 0, 0, framebuffer->width(), framebuffer->height() };
		m_pass.scissor = aka::Rect{ 0 };
		m_material->set<mat4f>("u_projection", projection);
		m_material->set<mat4f>("u_view", view);
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
		m_material->set<Texture::Ptr>("u_texture", batch.texture ? batch.texture : m_defaultTexture);
		m_pass.submesh.indexCount = batch.indexCount;
		m_pass.submesh.indexOffset = batch.indexOffset;
		m_pass.submesh.type = batch.primitive;
		m_pass.execute();
	}
}

};