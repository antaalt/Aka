#if defined(AKA_USE_D3D11)
#include "D3D11Blit.h"

#include "../D3D11Texture.h"

namespace aka {

void blitColor(const Texture::Ptr& src, const Framebuffer::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST, TextureFilter filter)
{
	// TODO compute pass ?
	const char* vertShader = ""
		"struct vs_out { float4 position : SV_POSITION; float2 texcoord : TEXCOORD; };\n"
		"vs_out main(float2 position : POSITION)\n"
		"{\n"
		"	vs_out output;\n"
		"	output.texcoord = position * 0.5 + 0.5;\n"
		"	output.texcoord.y = 1.f - output.texcoord.y;\n"
		"	output.position = float4(position.x, position.y, 0.0, 1.0);\n"
		"	return output;\n"
		"}\n";
	const char* fragShader = ""
		"struct vs_out { float4 position : SV_POSITION; float2 texcoord : TEXCOORD; };\n"
		"Texture2D    u_input : register(t0);\n"
		"SamplerState u_inputSampler : register(s0);\n"
		"float4 main(vs_out input) : SV_Target\n"
		"{\n"
		"	return u_input.Sample(u_inputSampler, input.texcoord);\n"
		"}\n";
	RenderPass pass;
	pass.framebuffer = dst;

	// Setup quad mesh
	float quadVertices[] = {
		-1.f, -1.f, // bottom left corner
		 1.f, -1.f, // bottom right corner
		 1.f,  1.f, // top right corner
		-1.f,  1.f, // top left corner
	};
	uint16_t quadIndices[] = { 0,1,2,0,2,3 };
	VertexAttribute quadAttributes = VertexAttribute{ VertexSemantic::Position, VertexFormat::Float, VertexType::Vec2 };
	pass.submesh.mesh = Mesh::create();
	pass.submesh.mesh->uploadInterleaved(&quadAttributes, 1, quadVertices, 4, IndexFormat::UnsignedShort, quadIndices, 6);
	pass.submesh.type = PrimitiveType::Triangles;
	pass.submesh.offset = 0;
	pass.submesh.count = 6;
	pass.material = Material::create(Program::createVertexProgram(
		Shader::compile(vertShader, ShaderType::Vertex),
		Shader::compile(fragShader, ShaderType::Fragment),
		&quadAttributes, 1
	));
	pass.clear = Clear::none;
	pass.blend = Blending::none;
	pass.cull = Culling::none;
	pass.depth = Depth::none;
	pass.stencil = Stencil::none;
	pass.scissor = Rect{ 0 };
	pass.viewport = Rect{ 0 };

	// Setup textures
	TextureSampler sampler;
	sampler.filterMin = filter;
	sampler.filterMag = filter;
	sampler.mipmapMode = TextureMipMapMode::None;
	sampler.wrapU = TextureWrap::ClampToEdge;
	sampler.wrapV = TextureWrap::ClampToEdge;
	sampler.wrapW = TextureWrap::ClampToEdge;
	sampler.anisotropy = 1.f;

	pass.material->set("u_input", sampler);
	pass.material->set("u_input", src);

	pass.execute();
}

void blitDepth(const Texture::Ptr& src, const Framebuffer::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST, TextureFilter filter)
{
	const char* vertShader = ""
		"struct vs_out { float4 position : SV_POSITION; float2 texcoord : TEXCOORD; };\n"
		"vs_out main(float2 position : POSITION)\n"
		"{\n"
		"	vs_out output;\n"
		"	output.texcoord = position * 0.5 + 0.5;\n"
		"	output.texcoord.y = 1.f - output.texcoord.y;\n"
		"	output.position = float4(position.x, position.y, 0.0, 1.0);\n"
		"	return output;\n"
		"}\n";
	const char* fragShader = ""
		"struct vs_out { float4 position : SV_POSITION; float2 texcoord : TEXCOORD; };\n"
		"Texture2D    u_input : register(t0);\n"
		"SamplerState u_inputSampler : register(s0);\n"
		"float main(vs_out input) : SV_Depth\n"
		"{\n"
		"	return u_input.Sample(u_inputSampler, input.texcoord).x;\n"
		"}\n";
	RenderPass pass;
	pass.framebuffer = dst;

	// Setup quad mesh
	float quadVertices[] = {
		-1.f, -1.f, // bottom left corner
		 1.f, -1.f, // bottom right corner
		 1.f,  1.f, // top right corner
		-1.f,  1.f, // top left corner
	};
	uint16_t quadIndices[] = { 0,1,2,0,2,3 };
	VertexAttribute quadAttributes = VertexAttribute{ VertexSemantic::Position, VertexFormat::Float, VertexType::Vec2 };
	pass.submesh.mesh = Mesh::create();
	pass.submesh.mesh->uploadInterleaved(&quadAttributes, 1, quadVertices, 4, IndexFormat::UnsignedShort, quadIndices, 6);
	pass.submesh.type = PrimitiveType::Triangles;
	pass.submesh.offset = 0;
	pass.submesh.count = 6;
	pass.material = Material::create(Program::createVertexProgram(
		Shader::compile(vertShader, ShaderType::Vertex),
		Shader::compile(fragShader, ShaderType::Fragment),
		&quadAttributes, 1
	));
	pass.clear = Clear::none;
	pass.blend = Blending::none;
	pass.cull = Culling::none;
	pass.depth = Depth::none;
	pass.stencil = Stencil::none;
	pass.scissor = Rect{ 0 };
	pass.viewport = Rect{ regionDST.x, regionDST.y, regionDST.width, regionDST.height };

	// Setup textures
	TextureSampler sampler;
	sampler.filterMin = filter;
	sampler.filterMag = filter;
	sampler.mipmapMode = TextureMipMapMode::None;
	sampler.wrapU = TextureWrap::ClampToEdge;
	sampler.wrapV = TextureWrap::ClampToEdge;
	sampler.wrapW = TextureWrap::ClampToEdge;
	sampler.anisotropy = 1.f;

	pass.material->set("u_input", sampler);
	pass.material->set("u_input", src);

	pass.execute();
}

};

#endif