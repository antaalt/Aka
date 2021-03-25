#include <Aka/Graphic/Batch2D.h>

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

Batch2D::Rect::Rect() :
	position(0.f),
	size(1.f),
	uv{ uv2f(0.f), uv2f(0.f), uv2f(0.f), uv2f(0.f) },
	color{ color4f(1.f), color4f(1.f), color4f(1.f), color4f(1.f) },
	texture(nullptr),
	layer(0)
{
}

Batch2D::Rect::Rect(const vec2f& pos, const vec2f& size, const color4f& color, int32_t layer) :
	position(pos),
	size(size),
	uv{ uv2f(0.f), uv2f(1.f, 0.f), uv2f(0.f, 1.f), uv2f(1.f) },
	color{ color, color, color, color },
	texture(nullptr),
	layer(layer)
{
}

Batch2D::Rect::Rect(const vec2f& pos, const vec2f& size, Texture::Ptr texture, int32_t layer) :
	position(pos),
	size(size),
	uv{ uv2f(0.f), uv2f(1.f, 0.f), uv2f(0.f, 1.f), uv2f(1.f) },
	color{ color4f(1.f), color4f(1.f), color4f(1.f), color4f(1.f) },
	texture(texture), 
	layer(layer)
{
}

Batch2D::Rect::Rect(const vec2f& pos, const vec2f& size, Texture::Ptr texture, const color4f& color, int32_t layer) :
	position(pos),
	size(size),
	uv{ uv2f(0.f), uv2f(1.f, 0.f), uv2f(0.f, 1.f), uv2f(1.f) },
	color{ color, color, color, color },
	texture(texture),
	layer(layer)
{
}

Batch2D::Rect::Rect(const vec2f& pos, const vec2f& size, const uv2f& uv0, const uv2f& uv1, Texture::Ptr texture, int32_t layer) :
	position(pos),
	size(size),
	uv{ uv0, uv2f(uv1.u, uv0.v), uv2f(uv0.u, uv1.v), uv1 },
	color{ color4f(1.f), color4f(1.f), color4f(1.f), color4f(1.f) },
	texture(texture),
	layer(layer)
{
}
Batch2D::Rect::Rect(const vec2f& pos, const vec2f& size, const uv2f& uv0, const uv2f& uv1, Texture::Ptr texture, const color4f& color, int32_t layer) :
	position(pos),
	size(size),
	uv{ uv0, uv2f(uv1.u, uv0.v), uv2f(uv0.u, uv1.v), uv1 },
	color{ color, color, color, color },
	texture(texture),
	layer(layer)
{
}

Batch2D::Rect::Rect(const vec2f& pos, const vec2f& size, const uv2f& uv0, const uv2f& uv1, const color4f& color0, const color4f& color1, const color4f& color2, const color4f& color3, int32_t layer) :
	position(pos),
	size(size),
	uv{uv0, uv2f(uv1.u, uv0.v), uv2f(uv0.u, uv1.v), uv1},
	color{color0, color1, color2, color3},
	texture(nullptr),
	layer(layer)

{
}

Batch2D::Text::Text() :
	Text("", nullptr, color4f(1.f), 0)
{
}
Batch2D::Text::Text(const std::string& str, Font* font, const color4f& color, int32_t layer) :
	text(str),
	font(font),
	color(color),
	layer(layer)
{
}

void Batch2D::draw(const mat3f& transform, Rect&& rect)
{
	DrawBatch& currentBatch = get(rect.texture, rect.layer);
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
	currentBatch.elements += 2;
}

void Batch2D::draw(const mat3f& transform, Tri&& tri)
{
	throw std::runtime_error("Not implemented");
}

void Batch2D::draw(const mat3f& transform, Quad&& quad)
{
	DrawBatch& currentBatch = get(quad.texture, quad.layer);
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
	currentBatch.elements += 2;
}

void Batch2D::draw(const mat3f& transform, Line&& line)
{
	throw std::runtime_error("Not implemented");
}

void Batch2D::draw(const mat3f& transform, Text&& text)
{
	float scale = 1.f;
	float advance = 0.f;
	String str = text.text;
	const char* start = str.begin();
	const char* end = str.end();
	while (start < end)
	{
		uint32_t c = encoding::next(start, end);
		// TODO check if rendering text out of screen for culling ?
		const Character& ch = text.font->getCharacter(c);
		vec2f position = vec2f(advance + ch.bearing.x, (float)-(ch.size.y - ch.bearing.y)) * scale;
		vec2f size = vec2f((float)ch.size.x, (float)ch.size.y) * scale;
		draw(transform, Batch2D::Rect(position, size, ch.texture.get(0), ch.texture.get(1), ch.texture.texture, text.color, text.layer));
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		advance += ch.advance * scale;
	}
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
		ShaderID(0),
		std::vector<Attributes>{ // HLSL only
			Attributes{ AttributeID(0), "POS" },
			Attributes{ AttributeID(0), "TEX" },
			Attributes{ AttributeID(0), "COL" }
		}
	);
	m_material = ShaderMaterial::create(m_shader);
	m_mesh = Mesh::create();
	uint8_t data[4] = { 255, 255, 255, 255 };
	Sampler sampler;
	sampler.filterMag = Sampler::Filter::Nearest;
	sampler.filterMin = Sampler::Filter::Nearest;
	sampler.wrapS = Sampler::Wrap::Clamp;
	sampler.wrapT = Sampler::Wrap::Clamp;
	m_defaultTexture = Texture::create(1, 1, Texture::Format::UnsignedByte, Texture::Component::RGBA, sampler);
	m_defaultTexture->upload(data);

	m_pass = {};
	m_pass.clear = Clear{ ClearMask::None, color4f(1.f), 1.f, 0 };
	m_pass.blend = Blending::nonPremultiplied();
	m_pass.cull = Culling{ CullMode::None, CullOrder::CounterClockWise };
	m_pass.depth = Depth{ DepthCompare::None, true };
	m_pass.stencil = Stencil::none();
	m_pass.mesh = m_mesh;
	m_pass.primitive = PrimitiveType::Triangles;
	m_pass.material = m_material;
}

void Batch2D::destroy()
{
}

Batch2D::DrawBatch& Batch2D::create(Texture::Ptr texture, int32_t layer)
{
	size_t offset = (m_batches.size() == 0) ? 0 : m_batches.back().elementOffset + m_batches.back().elements;
	m_batches.emplace_back();
	m_batches.back().texture = texture;
	m_batches.back().elementOffset = static_cast<uint32_t>(offset);
	m_batches.back().elements = 0;
	m_batches.back().layer = layer;
	return m_batches.back();
}

Batch2D::DrawBatch& Batch2D::get(Texture::Ptr texture, int32_t layer)
{
	if (m_batches.size() == 0)
		return create(texture, layer);
	if (m_batches.back().elements == 0)
	{
		m_batches.back().texture = texture;
		m_batches.back().layer = layer;
		return m_batches.back();
	}
	if (m_batches.back().layer != layer || m_batches.back().texture != texture)
		return create(texture, layer);
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
		m_material->set<Texture::Ptr>("u_texture", batch.texture ? batch.texture : m_defaultTexture);
		m_pass.indexCount = batch.elements * 3;
		m_pass.indexOffset = batch.elementOffset * 3;
		m_pass.execute();
	}
}

Batch2D::Vertex::Vertex(const vec2f& position, const uv2f& uv, const color4f& color) :
	position(position),
	uv(uv),
	color(color)
{
}

};