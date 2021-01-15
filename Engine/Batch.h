#pragma once

#include "Geometry.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "Mesh.h"
#include "RenderPass.h"

namespace aka {

struct Rect {

};

// Push render into batch so that we can easily insert new batch.
// Use a std::list in order to insert new created batch at the correct depth.
class Batch
{
public:
	Batch();
	~Batch();

	// Draw a rect
	void rect(const mat4f& transform, const Rect &rect, const color4f &color);

	// Draw a texture 
	void texture(const mat4f &transform, Texture::Ptr texture);

	// Render to default framebuffer
	void render();

	// Render to specified framebuffer
	void render(Framebuffer::Ptr framebuffer);
private:
	Shader m_shader;
	Mesh m_mesh;
private:
	struct Vertex {
		vec2f position;
		vec2f uvs;
		color4f color;
	};
private:
	std::vector<uint32_t> m_indices;
	std::vector<Vertex> m_vertices;
};

};