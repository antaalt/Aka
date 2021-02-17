#pragma once

#include <Aka/Core/Geometry.h>
#include <Aka/Core/Font.h>
#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Mesh.h>
#include <Aka/Graphic/RenderPass.h>
#include <Aka/Graphic/GraphicBackend.h>

namespace aka {

class Batch
{
public:
	struct Rect {
		Rect();
		Rect(const vec2f& pos, const vec2f& size, const color4f& color, int32_t layer);
		Rect(const vec2f& pos, const vec2f& size, Texture::Ptr texture, int32_t layer);
		Rect(const vec2f& pos, const vec2f& size, Texture::Ptr texture, const color4f &color, int32_t layer);
		Rect(const vec2f& pos, const vec2f& size, const uv2f& uv0, const uv2f& uv1, Texture::Ptr texture, int32_t layer);
		Rect(const vec2f& pos, const vec2f& size, const uv2f& uv0, const uv2f& uv1, Texture::Ptr texture, const color4f& color, int32_t layer);
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

	struct Text {
		Text();
		Text(const std::string &str, Font* font, const color4f& color, int32_t layer);
		std::string text;
		Font* font;
		color4f color;
		int32_t layer;
	};

	Batch();

	// Draw shapes
	void draw(const mat3f& transform, Rect&& rect);
	void draw(const mat3f& transform, Tri&& tri);
	void draw(const mat3f& transform, Quad&& quad);
	void draw(const mat3f& transform, Line&& line);
	void draw(const mat3f& transform, Text&& text);

	// Push the current batch to the stack and use a new one
	void push();

	// Push the current batch to the stack and use a new one with texture & layer set
	void push(Texture::Ptr texture, int32_t layer);

	// Clear all batch from stack
	void clear();

	// Get number of batch
	size_t batchCount() const;
	// Get number of vertices
	size_t verticesCount() const;
	// Get number of indices
	size_t indicesCount() const;

	// Render to specified framebuffer
	void render(Framebuffer::Ptr framebuffer, const mat4f& view, const mat4f &projection);
private:
	struct Vertex {
		vec2f position;
		uv2f uv;
		color4f color;
		Vertex(const vec2f& position, const uv2f& uv, const color4f& color);
	};

	struct DrawBatch {
		int32_t layer; // layer of the batch for reordering
		uint32_t elementOffset; // offset of indices in global array
		uint32_t elements; // number of triangles in the batch
		Texture::Ptr texture; // Texture of the batch 
	};

	std::vector<uint32_t> m_indices;
	std::vector<Vertex> m_vertices;
	Shader::Ptr m_shader; // Make static as we only need one instance of shader even though multiple class exist
	ShaderMaterial::Ptr m_material;
	Mesh::Ptr m_mesh;
	Texture::Ptr m_defaultTexture;

	DrawBatch m_currentBatch;
	std::vector<DrawBatch> m_batches;
};

};