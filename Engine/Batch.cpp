#include "Batch.h"

namespace aka {

Batch::Batch()
{
	// TODO create shader & mesh
}

Batch::~Batch()
{
}

void Batch::rect(const mat4f& transform, const Rect& rect, const color4f& color)
{
	size_t offset = m_vertices.size();
	m_indices.push_back(offset + 0);
	m_indices.push_back(offset + 1);
	m_indices.push_back(offset + 2);
	m_indices.push_back(offset + 3);
	Vertex v0;
	v0.position = vec2f(0.f, 1.f); // transform position
	v0.uvs = vec2f(0.f, 1.f);
	v0.color = color;
	Vertex v1;
	Vertex v2;
	Vertex v3;
	m_vertices.push_back(v0);
	m_vertices.push_back(v1);
	m_vertices.push_back(v2);
	m_vertices.push_back(v3);
}

void Batch::render(Framebuffer::Ptr framebuffer)
{
	RenderPass renderPass;

	renderPass.framebuffer = framebuffer.get();

	renderPass.mesh = &m_mesh;

	renderPass.shader = &m_shader;

	renderPass.execute();
}

};