#include <Aka/Graphic/Batch.h>

#include <algorithm>

namespace aka {

#if defined(AKA_USE_OPENGL)
// Move somewhere else
const char* vertShader =
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
const char* fragShader =
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
const char* shader = ""
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
const char* vertShader = shader;
const char* fragShader = shader;
#endif

Batch::Rect::Rect() :
	position(0.f),
	size(1.f),
	uv{ uv2f(0.f), uv2f(0.f), uv2f(0.f), uv2f(0.f) },
	color{ color4f(1.f), color4f(1.f), color4f(1.f), color4f(1.f) },
	texture(nullptr),
	layer(0)
{
}

Batch::Rect::Rect(const vec2f& pos, const vec2f& size, Texture::Ptr texture, int32_t layer) :
	position(pos),
	size(size),
	uv{ uv2f(0.f), uv2f(1.f, 0.f), uv2f(0.f, 1.f), uv2f(1.f) },
	color{ color4f(1.f), color4f(1.f), color4f(1.f), color4f(1.f) },
	texture(texture), 
	layer(layer)
{
}

Batch::Rect::Rect(const vec2f& pos, const vec2f& size, Texture::Ptr texture, const color4f& color, int32_t layer) :
	position(pos),
	size(size),
	uv{ uv2f(0.f), uv2f(1.f, 0.f), uv2f(0.f, 1.f), uv2f(1.f) },
	color{ color, color, color, color },
	texture(texture),
	layer(layer)
{
}

Batch::Rect::Rect(const vec2f& pos, const vec2f& size, const uv2f& uv0, const uv2f& uv1, Texture::Ptr texture, int32_t layer) :
	position(pos),
	size(size),
	uv{ uv0, uv2f(uv1.u, uv0.v), uv2f(uv0.u, uv1.v), uv1 },
	color{ color4f(1.f), color4f(1.f), color4f(1.f), color4f(1.f) },
	texture(texture),
	layer(layer)
{
}
Batch::Rect::Rect(const vec2f& pos, const vec2f& size, const uv2f& uv0, const uv2f& uv1, Texture::Ptr texture, const color4f& color, int32_t layer) :
	position(pos),
	size(size),
	uv{ uv0, uv2f(uv1.u, uv0.v), uv2f(uv0.u, uv1.v), uv1 },
	color{ color, color, color, color },
	texture(texture),
	layer(layer)
{
}

Batch::Rect::Rect(const vec2f& pos, const vec2f& size, const uv2f& uv0, const uv2f& uv1, const color4f& color0, const color4f& color1, const color4f& color2, const color4f& color3, int32_t layer) :
	position(pos),
	size(size),
	uv{uv0, uv2f(uv1.u, uv0.v), uv2f(uv0.u, uv1.v), uv1},
	color{color0, color1, color2, color3},
	texture(nullptr),
	layer(layer)

{
}

void Batch::draw(const mat3f& transform, Rect&& rect)
{
	if (m_currentBatch.layer != rect.layer) {
		if (m_currentBatch.elements > 0)
			push(rect.texture, rect.layer);
		else
			m_currentBatch.layer = rect.layer;
	}
	if (rect.texture != m_currentBatch.texture)
	{
		if (m_currentBatch.elements > 0)
			push(rect.texture, rect.layer);
		else
			m_currentBatch.texture = rect.texture;
	}
	uint32_t offset = static_cast<uint32_t>(m_vertices.size());
	m_indices.push_back(offset + 0);
	m_indices.push_back(offset + 1);
	m_indices.push_back(offset + 2);
	m_indices.push_back(offset + 2);
	m_indices.push_back(offset + 1);
	m_indices.push_back(offset + 3);
#if defined(ORIGIN_TOP_LEFT)
	for (size_t i = 0; i < 4; i++)
		rect.uv[i].v = 1.f - rect.uv[i].v;
#endif
	m_vertices.push_back(Vertex(transform.multiplyPoint(rect.position), rect.uv[0], rect.color[0])); // bottom left
	m_vertices.push_back(Vertex(transform.multiplyPoint(vec2f(rect.position.x + rect.size.x, rect.position.y)), rect.uv[1], rect.color[1])); // bottom right
	m_vertices.push_back(Vertex(transform.multiplyPoint(vec2f(rect.position.x, rect.position.y + rect.size.y)), rect.uv[2], rect.color[2])); // top left
	m_vertices.push_back(Vertex(transform.multiplyPoint(rect.position + rect.size), rect.uv[3], rect.color[3])); // top right
	m_currentBatch.elements += 2;
}

void Batch::draw(const mat3f& transform, Quad&& quad)
{
	if (m_currentBatch.layer != quad.layer) {
		if (m_currentBatch.elements > 0)
			push(quad.texture, quad.layer);
		else
			m_currentBatch.layer = quad.layer;
	}
	if (quad.texture != m_currentBatch.texture)
	{
		if (m_currentBatch.elements > 0)
			push(quad.texture, quad.layer);
		else
			m_currentBatch.texture = quad.texture;
	}
	uint32_t offset = static_cast<uint32_t>(m_vertices.size());
	m_indices.push_back(offset + 0);
	m_indices.push_back(offset + 1);
	m_indices.push_back(offset + 2);
	m_indices.push_back(offset + 2);
	m_indices.push_back(offset + 1);
	m_indices.push_back(offset + 3);
	m_vertices.push_back(Vertex(transform * quad.position[0], quad.uv[2], quad.color[0]));
	m_vertices.push_back(Vertex(transform * quad.position[1], quad.uv[3], quad.color[1]));
	m_vertices.push_back(Vertex(transform * quad.position[2], quad.uv[0], quad.color[2]));
	m_vertices.push_back(Vertex(transform * quad.position[3], quad.uv[1], quad.color[3]));
	m_currentBatch.elements += 2;
}

Batch::Batch() :
	m_shader(nullptr),
	m_mesh(nullptr),
	m_defaultTexture(nullptr)
{
	clear();
}

void Batch::push()
{
	push(m_defaultTexture, 0);
}

void Batch::push(Texture::Ptr texture, int32_t layer)
{
	if (m_defaultTexture == nullptr)
	{
		uint8_t data[4] = { 255, 255, 255, 255 };
		Sampler sampler;
		sampler.filterMag = Sampler::Filter::Nearest;
		sampler.filterMin = Sampler::Filter::Nearest;
		sampler.wrapS = Sampler::Wrap::Clamp;
		sampler.wrapT = Sampler::Wrap::Clamp;
		m_defaultTexture = Texture::create(1, 1, Texture::Format::Rgba, data, sampler);
	}
	m_batches.push_back(m_currentBatch);
	m_currentBatch.texture = texture ? texture : m_defaultTexture;
	m_currentBatch.elementOffset += m_currentBatch.elements;
	m_currentBatch.elements = 0;
	m_currentBatch.layer = layer;
}

void Batch::clear()
{
	m_batches.clear();
	m_vertices.clear();
	m_indices.clear();
	m_currentBatch.elementOffset = 0;
	m_currentBatch.elements = 0;
	m_currentBatch.texture = m_defaultTexture;
	m_currentBatch.layer = 0;
}

uint32_t Batch::count()
{
	if (m_currentBatch.elements > 0)
		return static_cast<uint32_t>(m_batches.size()) + 1;
	return static_cast<uint32_t>(m_batches.size());
}

void Batch::render(Framebuffer::Ptr framebuffer, const mat4f& view, const mat4f& projection)
{
	RenderPass renderPass {};
	{
		if (m_shader == nullptr)
		{
			m_shader = Shader::create(
				Shader::compile(vertShader, ShaderType::Vertex),
				Shader::compile(fragShader, ShaderType::Fragment),
				ShaderID(0),
				std::vector<Attributes>{ // HLSL only
					Attributes{ AttributeID(0), "POS"},
					Attributes{ AttributeID(0), "TEX" },
					Attributes{ AttributeID(0), "COL" }
				}
			);
			m_material = ShaderMaterial::create(m_shader);
		}
		if (m_mesh == nullptr)
			m_mesh = Mesh::create();
		m_material->set<mat4f>("u_projection", projection);
		m_material->set<mat4f>("u_view", view);
	}

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
		static const Blending blend = Blending::normal();
		static const Culling cull = Culling{ CullMode::None, CullOrder::CounterClockWise };
		static const Depth depth = Depth{ DepthCompare::None, true };
		static const Stencil stencil = Stencil::none();
		// Prepare renderPass
		renderPass.framebuffer = framebuffer;

		renderPass.mesh = m_mesh;

		renderPass.material = m_material;

		renderPass.blend = blend;

		renderPass.cull = cull;

		renderPass.depth = depth;

		renderPass.stencil = stencil;

		renderPass.viewport = aka::Rect{ 0, 0, framebuffer->width(), framebuffer->height() };

		renderPass.scissor = aka::Rect{ 0 };
	}
	
	// Don't forget last batch if there is something
	if(m_currentBatch.elements > 0)
		m_batches.push_back(m_currentBatch);

	// Sort batches by layer before rendering
	std::sort(m_batches.begin(), m_batches.end(), [](const DrawBatch &lhs, const DrawBatch &rhs) {
		return lhs.layer < rhs.layer;
	});

	// Draw the batches
	for (const DrawBatch &batch : m_batches)
	{
		m_material->set<Texture::Ptr>("u_texture", batch.texture);
		renderPass.indexCount = batch.elements * 3;
		renderPass.indexOffset = batch.elementOffset * 3;
		renderPass.execute();
	}
}

Batch::Vertex::Vertex(const vec2f& position, const uv2f& uv, const color4f& color) :
	position(position),
	uv(uv),
	color(color)
{
}

};