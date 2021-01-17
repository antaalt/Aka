#include "Batch.h"

namespace aka {

// Move somewhere else
const char* vertShader =
"#version 330\n"
"layout (location = 0) in vec2 a_position;\n"
"layout (location = 1) in vec2 a_uv;\n"
"layout (location = 2) in vec4 a_color;\n"
"uniform mat4 u_matrix;\n"
"out vec2 v_uv;\n"
"out vec4 v_color;\n"
"void main(void) {\n"
"	gl_Position = u_matrix * vec4(a_position.xy, 0.0, 1.0);\n"
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
"	o_color = v_color *texture(u_texture, v_uv);\n"
"}"
"";

Batch::Batch() :
	m_mesh(Mesh::create())
{
	ShaderInfo info{};
	info.vertex = Shader::create(vertShader, ShaderType::VERTEX_SHADER);
	info.frag = Shader::create(fragShader, ShaderType::FRAGMENT_SHADER);
	info.uniforms.push_back(Uniform{ UniformType::Mat4, ShaderType::VERTEX_SHADER, "u_matrix" });
	m_shader.create(info);

	uint8_t data[4] = { 255, 255, 255, 255 };
	m_defaultTexture = Texture::create(1, 1, Texture::Format::Rgba8, Texture::Format::Rgba, data, Sampler::Filter::Nearest);

	clear();
}

Batch::~Batch()
{
	m_shader.destroy();
}

void Batch::rect(const mat3f& transform, const aka::Rect& rect, const color4f& color)
{
	this->rect(transform, vec2f(rect.x, rect.y), vec2f(rect.x + rect.w, rect.y + rect.h), vec2f(0.f, 0.f), vec2f(1.f, 1.f), color, color, color, color);
}

void Batch::rect(const mat3f& transform, const vec2f& pos0, const vec2f& pos1, const color4f& color)
{
	rect(transform, pos0, pos1, vec2f(0.f), vec2f(1.f), color, color, color, color);
}

void Batch::rect(const mat3f& transform, const vec2f& pos0, const vec2f& pos1, const vec2f& uv0, const vec2f& uv1, const color4f& color)
{
	rect(transform, pos0, pos1, uv0, uv1, color, color, color, color);
}

void Batch::rect(const mat3f& transform, const vec2f& pos0, const vec2f& pos1, const vec2f& uv0, const vec2f& uv1, const color4f& color0, const color4f& color1, const color4f& color2, const color4f& color3)
{
	quad(transform, pos0, vec2f(pos1.x, pos0.y), vec2f(pos0.x, pos1.y), pos1, uv0, vec2f(uv1.x, uv0.y), vec2f(uv0.x, uv1.y), uv1, color0, color1, color2, color3);
}

void Batch::quad(const mat3f& transform, const vec2f& pos0, const vec2f& pos1, const vec2f& pos2, const vec2f& pos3, const vec2f& uv0, const vec2f& uv1, const vec2f& uv2, const vec2f& uv3, const color4f& color0, const color4f& color1, const color4f& color2, const color4f& color3)
{
	uint32_t offset = static_cast<uint32_t>(m_vertices.size());
	m_indices.push_back(offset + 0);
	m_indices.push_back(offset + 1);
	m_indices.push_back(offset + 2);
	m_indices.push_back(offset + 2);
	m_indices.push_back(offset + 1);
	m_indices.push_back(offset + 3);
	m_vertices.push_back(Vertex(transform * pos0, uv0, color0));
	m_vertices.push_back(Vertex(transform * pos1, uv1, color1));
	m_vertices.push_back(Vertex(transform * pos2, uv2, color2));
	m_vertices.push_back(Vertex(transform * pos3, uv3, color3));
	m_currentBatch.elements += 2;
}

void Batch::texture(const mat3f& transform, const vec2f& position, const vec2f& size, Texture::Ptr texture)
{
	this->texture(transform, position, size, texture, color4f(1.f));
}

void Batch::texture(const mat3f& transform, const vec2f& position, const vec2f& size, const uv2f& start, const uv2f& end, Texture::Ptr texture)
{
	if (m_currentBatch.elements > 0 && m_currentBatch.texture != texture && m_currentBatch.texture != nullptr)
		push();
	m_currentBatch.texture = texture;
	rect(transform, position, position + size, vec2f(start), vec2f(end), color4f(1.f));
}

void Batch::texture(const mat3f& transform, const vec2f& position, const vec2f& size, Texture::Ptr texture, const color4f& color)
{
	if (m_currentBatch.elements > 0 && m_currentBatch.texture != texture && m_currentBatch.texture != nullptr)
		push();
	m_currentBatch.texture = texture;
	rect(transform, aka::Rect{ position.x, position.y, size.x, size.y }, color);
}

void Batch::push()
{
	m_batches.push_back(m_currentBatch);
	m_currentBatch.texture = m_defaultTexture;
	m_currentBatch.elementOffset += m_currentBatch.elements;
	m_currentBatch.elements = 0;
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

void Batch::render(Framebuffer::Ptr framebuffer)
{
	RenderPass renderPass {};
	{
		// Update mesh data
		m_mesh->indices(IndexFormat::Uint32, m_indices.data(), m_indices.size());
		VertexData data;
		data.attributes.push_back(VertexData::Attribute{ 0, VertexFormat::Float2 });
		data.attributes.push_back(VertexData::Attribute{ 1, VertexFormat::Float2 });
		data.attributes.push_back(VertexData::Attribute{ 2, VertexFormat::Float4 });
		m_mesh->vertices(data, m_vertices.data(), m_vertices.size());
	}
	{
		// TODO build matrix out of here to support more projection type
		m_shader.use();
		m_shader.set<mat4f>("u_matrix", mat4f::orthographic(0.f, static_cast<float>(framebuffer->height()), 0.f, static_cast<float>(framebuffer->width()), -1.f, 1.f));
	}

	{
		// Prepare renderPass
		renderPass.framebuffer = framebuffer;

		renderPass.mesh = m_mesh;

		renderPass.shader = &m_shader;

		renderPass.blend = BlendMode::OneMinusSrcAlpha;

		renderPass.cull = CullMode::None;

		renderPass.viewport = aka::Rect{ 0.f, 0.f, static_cast<float>(framebuffer->width()), static_cast<float>(framebuffer->height()) };
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
		renderPass.indexCount = batch.elements * 3;
		renderPass.indexOffset = batch.elementOffset * 3;
		renderPass.texture = batch.texture;
		renderPass.execute();
	}
	// Clear all batches as they are rendered
	m_batches.clear();
}

Batch::Vertex::Vertex(const vec2f& position, const vec2f& uv, const color4f& color) :
	position(position),
	uv(uv),
	color(color)
{
}

};