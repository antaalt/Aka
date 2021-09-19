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

class Batch2D
{
public:
	struct Vertex {
		vec2f position;
		uv2f uv;
		color4f color;
	};

	struct Line {
		Vertex vertices[2];
		int32_t layer;
	};

	struct Triangle {
		Vertex vertices[3];
		Texture::Ptr texture;
		int32_t layer;
	};

	struct Quad {
		Vertex vertices[4];
		Texture::Ptr texture;
		int32_t layer;
	};

	struct Poly {
		std::vector<Vertex> vertices;
		Texture::Ptr texture;
		PrimitiveType primitive;
		int32_t layer;
	};

public:
	Batch2D();
	~Batch2D();

	// Initialize batch 
	void initialize();
	// Destroy batch 
	void destroy();

	// Draw Line
	void draw(const mat3f& transform, const Line& line);
	// Draw Triangle
	void draw(const mat3f& transform, const Triangle& tri);
	// Draw Quad
	void draw(const mat3f& transform, const Quad& quad);
	// Draw Poly
	void draw(const mat3f& transform, const Poly& poly);

	// Clear all batch from stack
	void clear();

	// Get number of batch
	size_t batchCount() const;
	// Get number of vertices
	size_t verticesCount() const;
	// Get number of indices
	size_t indicesCount() const;

	// Render to backbuffer
	void render();
	// Render to specified framebuffer
	void render(Framebuffer::Ptr framebuffer);
	// Render to specified framebuffer with transform
	void render(Framebuffer::Ptr framebuffer, const mat4f& view);
	// Render to specified framebuffer with transform & projection
	void render(Framebuffer::Ptr framebuffer, const mat4f& view, const mat4f &projection);
private:

	struct DrawBatch {
		int32_t layer; // layer of the batch for reordering
		uint32_t indexOffset; // offset of indices in global array
		uint32_t indexCount; // number of triangles in the batch
		PrimitiveType primitive; // Type of the primitive
		Texture::Ptr texture; // Texture of the batch 
	};

	// Create the batch to the stack and use a new one with texture & layer set
	DrawBatch& create(PrimitiveType type, Texture::Ptr texture, int32_t layer);
	// Get the batch with settings
	DrawBatch& get(PrimitiveType type, Texture::Ptr texture, int32_t layer);

private:
	std::vector<uint32_t> m_indices;
	std::vector<Vertex> m_vertices;
	uint32_t m_maxVertices;
	uint32_t m_maxIndices;
	RenderPass m_pass;
	Program::Ptr m_program; // Make static as we only need one instance of shader even though multiple class exist
	Material::Ptr m_material;
	Buffer::Ptr m_uniformBuffer;
	Mesh::Ptr m_mesh;
	Buffer::Ptr m_vertexBuffer;
	Buffer::Ptr m_indexBuffer;
	Texture::Ptr m_defaultTexture;

	std::vector<DrawBatch> m_batches;
};

};