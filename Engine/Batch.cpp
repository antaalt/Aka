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

	clear();
}

Batch::~Batch()
{
	m_shader.destroy();
}

void Batch::quad(const mat3f& transform, float x0, float y0, float x1, float y1, float uvx0, float uvy0, float uvx1, float uvy1, float r, float g, float b, float a)
{
	uint32_t offset = static_cast<uint32_t>(m_vertices.size());
	m_indices.push_back(offset + 0);
	m_indices.push_back(offset + 1);
	m_indices.push_back(offset + 2);
	m_indices.push_back(offset + 2);
	m_indices.push_back(offset + 1);
	m_indices.push_back(offset + 3);
	m_vertices.push_back(Vertex(transform * vec2f(x0, y0), vec2f(uvx0, uvy0), color4f(r, g, b, a)));
	m_vertices.push_back(Vertex(transform * vec2f(x1, y0), vec2f(uvx1, uvy0), color4f(r, g, b, a)));
	m_vertices.push_back(Vertex(transform * vec2f(x0, y1), vec2f(uvx0, uvy1), color4f(r, g, b, a)));
	m_vertices.push_back(Vertex(transform * vec2f(x1, y1), vec2f(uvx1, uvy1), color4f(r, g, b, a)));
	m_currentBatch.elements += 2;
}

void Batch::rect(const mat3f& transform, const Rect& rect, const color4f& color)
{
	quad(transform, rect.x, rect.y, rect.w + rect.x, rect.h + rect.y, 0.f, 0.f, 1.f, 1.f, color.r, color.g, color.b, color.a);
}

void Batch::texture(const mat3f& transform, Texture::Ptr texture)
{
	// update texture stack & create new drawbatch
	if (m_currentBatch.elements > 0 && m_currentBatch.texture != texture && m_currentBatch.texture != nullptr)
	{
		m_batches.push_back(m_currentBatch);
		m_currentBatch.texture = texture;
		Rect rectangle;
		rect(transform, rectangle, color4f(1.f));
	}
	else
	{
		m_currentBatch.texture = texture;
		Rect rectangle{ 0, 0, 50, 50};
		rect(transform, rectangle, color4f(1.f));
	}
}

void Batch::push()
{
	m_batches.push_back(m_currentBatch);
	m_currentBatch.depth = 0.f;
	m_currentBatch.texture = nullptr;
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
	m_currentBatch.texture = nullptr;
	m_currentBatch.depth = 0.f;
}

void Batch::render()
{
	render(nullptr);
}

void Batch::render(Framebuffer::Ptr framebuffer)
{
	// TODO retrieve app dimensions instead of hard written one
	float w = 1280.f;
	float h = 720.f;

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
		// Prepare material
		float width, height;
		if (framebuffer)
		{
			width = static_cast<float>(framebuffer->width());
			height = static_cast<float>(framebuffer->height());
		}
		else
		{
			width = w;
			height = h;
		}
		// TODO build matrix out of here to support more projection type
		m_shader.use();
		m_shader.set<mat4f>("u_matrix", mat4f::orthographic(0.f, height, 0.f, width, -1.f, 1.f));
	}

	{
		// Prepare renderPass
		renderPass.framebuffer = framebuffer;

		renderPass.mesh = m_mesh;

		renderPass.shader = &m_shader;

		renderPass.blend = BlendMode::OneMinusSrcAlpha;

		renderPass.cull = CullMode::None;

		renderPass.texture = m_currentBatch.texture;

		renderPass.viewport = Rect{ 0.f, 0.f, w, h };
	}
	
	// Don't forget last batch if there is something
	if(m_currentBatch.elements > 0)
		m_batches.push_back(m_currentBatch);

	// Draw the batches
	for (DrawBatch &batch : m_batches)
	{
		renderPass.indexCount = batch.elements * 3;
		renderPass.indexOffset = batch.elementOffset;
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