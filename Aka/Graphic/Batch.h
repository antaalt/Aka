#pragma once

#include "../Core/Geometry.h"
#include "../Graphic/Shader.h"
#include "../Graphic/Framebuffer.h"
#include "../Graphic/Texture.h"
#include "../Graphic/Mesh.h"
#include "../Graphic/RenderPass.h"
#include "../Graphic/GraphicBackend.h"

namespace aka {

class Batch
{
public:
	struct Rect {
		Rect();
		Rect(const vec2f& pos, const vec2f& size, Texture::Ptr texture, int32_t layer);
		Rect(const vec2f& pos, const vec2f& size, Texture::Ptr texture, const color4f &color, int32_t layer);
		Rect(const vec2f& pos, const vec2f& size, const uv2f& uv0, const uv2f& uv1, Texture::Ptr texture, int32_t layer);
		Rect(const vec2f& pos, const vec2f& size, const uv2f& uv0, const uv2f& uv1, const color4f& color0, const color4f& color1, const color4f& color2, const color4f& color3, int32_t layer);
		vec2f position;
		vec2f size;
		uv2f uv[4];
		color4f color[4];
		Texture::Ptr texture;
		int32_t layer;
	};
	struct Tri {
		Tri();
		vec2f position[3];
		uv2f uv[3];
		uv2f color[3];
		Texture::Ptr texture;
		int32_t layer;
	};
	struct Quad {
		Quad();
		Quad(Texture::Ptr t, const vec2f& p0, const vec2f& p1, const vec2f& p2, const vec2f& p3, const uv2f& u0, const uv2f& u1, const uv2f& u2, const uv2f& u3, const color4f& c0, const color4f& c1, const color4f& c2, const color4f& c3);
		
		vec2f position[4];
		uv2f uv[4];
		color4f color[4];
		Texture::Ptr texture;
		int32_t layer;
	};
	struct Line {
		Line();
		vec2f position[2];
		color4f color[2];
		int32_t layer;
	};

	Batch();

	// Draw shapes
	void draw(const mat3f& transform, Rect&& rect);
	void draw(const mat3f& transform, Tri&& tri);
	void draw(const mat3f& transform, Quad&& quad);
	void draw(const mat3f& transform, Line&& line);

	// Push the current batch to the stack and use a new one
	void push();

	// Push the current batch to the stack and use a new one with texture & layer set
	void push(Texture::Ptr texture, int32_t layer);

	// Clear all batch from stack
	void clear();

	// Render to specified framebuffer
	void render(Framebuffer::Ptr framebuffer, const mat4f &projection);
private:
	struct Vertex {
		vec2f position;
		uv2f uv;
		color4f color;
		Vertex(const vec2f& position, const uv2f& uv, const color4f& color);
	};

	struct DrawBatch {
		int32_t layer; // layer of the batch for reordering
		size_t elementOffset; // offset of indices in global array
		size_t elements; // number of triangles in the batch
		Texture::Ptr texture; // Texture of the batch 
	};

	std::vector<uint32_t> m_indices;
	std::vector<Vertex> m_vertices;
	Shader::Ptr m_shader; // Make static as we only need one instance of shader even though multiple class exist
	Mesh::Ptr m_mesh;
	Texture::Ptr m_defaultTexture;

	DrawBatch m_currentBatch;
	std::vector<DrawBatch> m_batches;
};

};