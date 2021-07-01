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

class Batch3D
{
public:
	struct Vertex {
		point3f position;
		norm3f normal;
		uv2f uv;
		color4f color;
	};

	struct Line {
		Vertex vertices[2];
		Texture::Ptr texture;
	};

	struct Triangle {
		Vertex vertices[3];
		Texture::Ptr texture;
	};

	struct Quad {
		Vertex vertices[4];
		Texture::Ptr texture;
	};

	struct Poly {
		std::vector<Vertex> vertices;
		Texture::Ptr texture;
		PrimitiveType primitive;
	};

public:
	Batch3D();
	~Batch3D();

	// Initialize batch
	void initialize();
	// Destroy batch
	void destroy();

	// Draw triangle
	void draw(const mat4f& transform, const Triangle& triangle);
	// Draw quad
	void draw(const mat4f& transform, const Quad& quad);
	// Draw line
	void draw(const mat4f& transform, const Line& line);
	// Draw polygon
	void draw(const mat4f& transform, const Poly& poly);

	// Clear all batch from stack
	void clear();

	// Render to specified framebuffer with view & projection
	void render(Framebuffer::Ptr framebuffer, const mat4f& view, const mat4f& projection);
private:
	struct DrawBatch {
		uint32_t indexOffset; // offset of indices in global array
		uint32_t indexCount; // number of indices in the batch
		Texture::Ptr texture; // Texture of the batch 
		PrimitiveType primitive; // Primitive of the batch

		bool operator==(const DrawBatch& batch) const;
	};

	// Create the batch to the stack and use a new one with texture & layer set
	DrawBatch& create(PrimitiveType primitive, Texture::Ptr texture);
	// Get the batch with settings
	DrawBatch& get(PrimitiveType primitive, Texture::Ptr texture);

private:
	std::vector<uint32_t> m_indices;
	std::vector<Vertex> m_vertices;
	RenderPass m_pass;
	Shader::Ptr m_shader; // Make static as we only need one instance of shader even though multiple class exist
	ShaderMaterial::Ptr m_material;
	Mesh::Ptr m_mesh;
	Texture::Ptr m_defaultTexture;

	std::vector<DrawBatch> m_batches;
};

};