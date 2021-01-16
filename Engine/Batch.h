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
	
	// Draw a tri
	void tri(const vec2f& pos0, const vec2f& pos1, const vec2f& pos2, const color4f& color);

	// Draw a line
	void line(const mat3f& transform, const vec2f& from, const vec2f& to, float t, const color4f& color);

	// Draw a rect on the screen
	void rect(const mat3f& transform, const Rect& rect, const color4f& color);
	void rect(const mat3f& transform, const vec2f& pos0, const vec2f& pos1, const color4f& color);
	void rect(const mat3f& transform, const vec2f& pos0, const vec2f& pos1, const vec2f& uv0, const vec2f& uv1, const color4f& color);
	void rect(const mat3f& transform, const vec2f& pos0, const vec2f& pos1, const vec2f& uv0, const vec2f& uv1, const color4f& color0, const color4f& color1, const color4f& color2, const color4f& color3);

	// Draw a quad on the screen
	void quad(const mat3f &transform, const vec2f &pos0, const vec2f &pos1, const vec2f &pos2, const vec2f& pos3, const vec2f &uv0, const vec2f &uv1, const vec2f& uv2, const vec2f& uv3, const color4f&color0, const color4f&color1, const color4f&color2, const color4f&color3);

	// Draw a texture
	void texture(const mat3f& transform, const vec2f& position, const vec2f& size, Texture::Ptr texture);
	void texture(const mat3f& transform, const vec2f& position, const vec2f& size, Texture::Ptr texture, const color4f &color);

	// Push the current batch to the stack and use a new one
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
		int layer; // layer of the batch for reordering
		size_t elementOffset; // offset of indices in global array
		size_t elements; // number of triangles in the batch
		Texture::Ptr texture; // Texture of the batch 
	};

	std::vector<uint32_t> m_indices;
	std::vector<Vertex> m_vertices;
	Shader m_shader; // Make static as we only need one instance of shader even though multiple class exist
	Mesh::Ptr m_mesh;
	Texture::Ptr m_defaultTexture;

	DrawBatch m_currentBatch;
	std::vector<DrawBatch> m_batches;
};

};