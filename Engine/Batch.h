#pragma once

#include "Geometry.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "Mesh.h"
#include "RenderPass.h"
#include "GraphicBackend.h"

namespace aka {

class Batch
{
public:
	struct Rect {
		Rect();
		Rect(const vec2f& pos0, const vec2f& pos1, const uv2f& uv0, const uv2f& uv1, const color4f& color0, const color4f& color1, const color4f& color2, const color4f& color3);
		vec2f position;
		vec2f size;
		uv2f uv[4];
		uv2f color[4];
		Texture::Ptr texture;
	};
	struct Tri {
		Tri();
		vec2f position[3];
		uv2f uv[3];
		uv2f color[3];
		Texture::Ptr texture;
	};
	struct Quad {
		Quad();
		Quad(Texture::Ptr t, const vec2f& p0, const vec2f& p1, const vec2f& p2, const vec2f& p3, const uv2f& u0, const uv2f& u1, const uv2f& u2, const uv2f& u3, const color4f& c0, const color4f& c1, const color4f& c2, const color4f& c3);
		
		vec2f position[4];
		uv2f uv[4];
		color4f color[4];
		Texture::Ptr texture;
	};
	struct Line {
		Line();
		vec2f position[2];
		color4f color[2];
	};
	struct SubTexture {

	};
	void draw(const mat3f& transform, Rect&& rect);
	void draw(const mat3f& transform, Tri&& tri);
	void draw(const mat3f& transform, Quad&& quad);
	void draw(const mat3f& transform, Line&& line);
	void draw(const mat3f& transform, SubTexture&& texture);
public:
	Batch();
	~Batch();
	
	// Draw a tri
	void tri(const vec2f& pos0, const vec2f& pos1, const vec2f& pos2, const color4f& color);

	// Draw a line
	void line(const mat3f& transform, const vec2f& from, const vec2f& to, float t, const color4f& color);

	// Draw a rect on the screen
	void rect(const mat3f& transform, const aka::Rect& rect, const color4f& color);
	void rect(const mat3f& transform, const vec2f& pos0, const vec2f& pos1, const color4f& color);
	void rect(const mat3f& transform, const vec2f& pos0, const vec2f& pos1, const vec2f& uv0, const vec2f& uv1, const color4f& color);
	void rect(const mat3f& transform, const vec2f& pos0, const vec2f& pos1, const vec2f& uv0, const vec2f& uv1, const color4f& color0, const color4f& color1, const color4f& color2, const color4f& color3);

	// Draw a quad on the screen
	void quad(const mat3f &transform, const vec2f &pos0, const vec2f &pos1, const vec2f &pos2, const vec2f& pos3, const vec2f &uv0, const vec2f &uv1, const vec2f& uv2, const vec2f& uv3, const color4f&color0, const color4f&color1, const color4f&color2, const color4f&color3);

	// Draw a texture
	void texture(const mat3f& transform, const vec2f& position, const vec2f& size, Texture::Ptr texture);
	void texture(const mat3f& transform, const vec2f& position, const vec2f& size, const uv2f &start, const uv2f &end, Texture::Ptr texture);
	void texture(const mat3f& transform, const vec2f& position, const vec2f& size, Texture::Ptr texture, const color4f &color);

	// Push the current batch to the stack and use a new one
	void push();

	// Clear all batch from stack
	void clear();

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