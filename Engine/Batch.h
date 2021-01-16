#pragma once

#include "Geometry.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "Mesh.h"
#include "RenderPass.h"

namespace aka {

// Push render into batch so that we can easily insert new batch.
// Use a std::list in order to insert new created batch at the correct depth.
class Batch
{
public:
	Batch();
	~Batch();

	// Draw a quad on the screen
	void quad(const mat3f &transform, float x0, float y0, float x1, float y1, float uvx0, float uvy0, float uvx1, float uvx2, float r, float g, float b, float a);

	// Draw a rect
	void rect(const mat3f& transform, const Rect &rect, const color4f &color);

	// Draw a texture 
	void texture(const mat3f& transform, Texture::Ptr texture);

	// Push the current batch and use a new one
	void push();

	// Clear all batch from stack
	void clear();

	// Render to default framebuffer
	void render();

	// Render to specified framebuffer
	void render(Framebuffer::Ptr framebuffer);
private:
	struct Vertex {
		vec2f position;
		vec2f uv;
		color4f color;
		Vertex(const vec2f& position, const vec2f& uv, const color4f& color);
	};

	struct DrawBatch {
		float depth; // depth of the batch for reordering
		size_t elementOffset; // offset of indices in global array
		size_t elements; // number of triangles in the batch
		Texture::Ptr texture; // Texture of the batch 
	};

	std::vector<uint32_t> m_indices;
	std::vector<Vertex> m_vertices;
	Shader m_shader; // Make static as we only need one instance of shader even though multiple class exist
	Mesh::Ptr m_mesh;

	DrawBatch m_currentBatch;
	std::vector<DrawBatch> m_batches;
};

};