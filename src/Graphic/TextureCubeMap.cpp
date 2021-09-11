#include <Aka/Graphic/TextureCubeMap.h>

#include <Aka/Graphic/GraphicBackend.h>

namespace aka {

TextureCubeMap::TextureCubeMap(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags) :
	Texture(width, height, 1, TextureType::TextureCubeMap, format, flags)
{
}

TextureCubeMap::~TextureCubeMap()
{
}

TextureCubeMap::Ptr TextureCubeMap::create(
	uint32_t width, uint32_t height, 
	TextureFormat format, TextureFlag flags,
	const void* px, const void* nx,
	const void* py, const void* ny,
	const void* pz, const void* nz
)
{
	return GraphicBackend::createTextureCubeMap(width, height, format, flags, px, nx, py, ny, pz, nz);
}

TextureCubeMap::Ptr TextureCubeMap::generate(
	uint32_t width,
	uint32_t height,
	TextureFormat format,
	TextureFlag flags,
	Texture2D::Ptr equirectangular,
	TextureFilter filter
)
{
#if defined(AKA_USE_OPENGL)
	static const char* s_vertShader = "#version 330 core\n"
		"layout(location = 0) in vec3 a_position;\n"
		"layout(std140) uniform CameraUniformBuffer { mat4 projection; mat4 view; };\n"
		"out vec3 v_position;\n"
		"void main()\n"
		"{\n"
		"	v_position = a_position;\n"
		"	gl_Position = projection * view * vec4(a_position, 1.0);\n"
		"}\n";

	static const char* s_fragShader = "#version 330 core\n"
		"in vec3 v_position;\n"
		"uniform sampler2D u_equirectangularMap;\n"
		"out vec4 o_color;\n"
		"const vec2 invAtan = vec2(0.1591, 0.3183);\n"
		"vec2 SampleSphericalMap(vec3 v)\n"
		"{\n"
		"	vec2 uv = vec2(atan(v.z, v.x), asin(-v.y));\n"
		"	uv *= invAtan;\n"
		"	uv += 0.5;\n"
		"	return uv;\n"
		"}\n"
		"void main()\n"
		"{\n"
		"	vec2 uv = SampleSphericalMap(normalize(v_position));\n"
		"	vec3 color = texture(u_equirectangularMap, uv).rgb;\n"
		"	o_color = vec4(color, 1.0);\n"
		"}\n";
#elif defined(AKA_USE_D3D11)
	static const char* s_shader = ""
		"cbuffer CameraUniformBuffer : register(b0) { float4x4 projection; float4x4 view; };\n"
		"struct vs_out { float4 position : SV_POSITION; float3 localPosition : POS; };\n"
		"Texture2D    u_equirectangularMap : register(t0);\n"
		"SamplerState u_equirectangularMapSampler : register(s0);\n"
		"static const float2 invAtan = float2(0.1591, 0.3183);\n"
		"float2 SampleSphericalMap(float3 v)\n"
		"{\n"
		"	float2 uv = float2(atan2(v.z, v.x), asin(v.y));\n"
		"	uv *= invAtan;\n"
		"	uv += 0.5;\n"
		"	return uv;\n"
		"}\n"
		"vs_out vs_main(float3 position : POS)\n"
		"{\n"
		"	vs_out output;\n"
		"	output.localPosition = position;\n"
		"	output.position = mul(projection, mul(view, float4(position.x, position.y, position.z, 1.0)));\n"
		"	return output;\n"
		"}\n"
		"float4 ps_main(vs_out input) : SV_TARGET\n"
		"{\n"
		"	float2 uv = SampleSphericalMap(normalize(input.localPosition));\n"
		"	float3 color = u_equirectangularMap.Sample(u_equirectangularMapSampler, uv).rgb;\n"
		"	return float4(color.x, color.y, color.z, 1.0);\n"
		"}\n";
	static const char* s_vertShader = s_shader;
	static const char* s_fragShader = s_shader;
#endif
	RenderPass pass;
	// Setup framebuffer
	// TODO copy this cubemap to a cubemap without forced rendertarget flag
	TextureCubeMap::Ptr cubemap = TextureCubeMap::create(width, height, format, flags | TextureFlag::RenderTarget);
	Attachment attachment = Attachment{ AttachmentType::Color0, cubemap, AttachmentFlag::None, 0, 0 };
	pass.framebuffer = Framebuffer::create(&attachment, 1);

	// Setup cube mesh
	float skyboxVertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};
	pass.submesh.mesh = Mesh::create();
	uint32_t vertStride = sizeof(float) * 3;
	uint32_t vertCount = sizeof(skyboxVertices) / vertStride;
	VertexAccessor skyboxVertexInfo = {
		VertexAttribute{ VertexSemantic::Position, VertexFormat::Float, VertexType::Vec3 },
		VertexBufferView{
			Buffer::create(BufferType::Vertex, sizeof(skyboxVertices), BufferUsage::Immutable, BufferCPUAccess::None, skyboxVertices),
			0, // offset
			sizeof(skyboxVertices), // size
			vertStride // stride
		},
		0, // offset
		vertCount // count
	};
	pass.submesh.mesh->upload(&skyboxVertexInfo, 1);
	pass.submesh.type = PrimitiveType::Triangles;
	pass.submesh.offset = 0;
	pass.submesh.count = vertCount;

	// Setup shader
	VertexAttribute attribute{ VertexSemantic::Position, VertexFormat::Float, VertexType::Vec3 };
	ShaderHandle vert = Shader::compile(s_vertShader, ShaderType::Vertex);
	ShaderHandle frag = Shader::compile(s_fragShader, ShaderType::Fragment);
	pass.material = ShaderMaterial::create(Shader::createVertexProgram(vert, frag, &attribute, 1));
	Shader::destroy(vert);
	Shader::destroy(frag);

	// Setup uniforms
	struct CameraUniformBuffer {
		mat4f projection;
		mat4f view;
	};
	Buffer::Ptr uniforms = Buffer::create(BufferType::Uniform, sizeof(CameraUniformBuffer), BufferUsage::Default, BufferCPUAccess::None);

	// Setup pass
	pass.clear = Clear{ ClearMask::Color | ClearMask::Depth, color4f(0.f), 1.f, 0 };
	pass.blend = Blending::none;
	pass.cull = Culling::none;
	pass.depth = Depth::none;
	pass.stencil = Stencil::none;
	pass.scissor = Rect{ 0 };

	// Setup uniforms
	CameraUniformBuffer camera;
	camera.projection = mat4f::perspective(anglef::degree(90.0f), 1.f, 0.1f, 10.f);
	mat4f captureViews[6] = {
		mat4f::inverse(mat4f::lookAt(point3f(0.0f, 0.0f, 0.0f), point3f(1.0f,  0.0f,  0.0f), norm3f(0.0f, -1.0f,  0.0f))),
		mat4f::inverse(mat4f::lookAt(point3f(0.0f, 0.0f, 0.0f), point3f(-1.0f,  0.0f,  0.0f), norm3f(0.0f, -1.0f,  0.0f))),
		mat4f::inverse(mat4f::lookAt(point3f(0.0f, 0.0f, 0.0f), point3f(0.0f,  1.0f,  0.0f), norm3f(0.0f,  0.0f,  1.0f))),
		mat4f::inverse(mat4f::lookAt(point3f(0.0f, 0.0f, 0.0f), point3f(0.0f, -1.0f,  0.0f), norm3f(0.0f,  0.0f, -1.0f))),
		mat4f::inverse(mat4f::lookAt(point3f(0.0f, 0.0f, 0.0f), point3f(0.0f,  0.0f,  1.0f), norm3f(0.0f, -1.0f,  0.0f))),
		mat4f::inverse(mat4f::lookAt(point3f(0.0f, 0.0f, 0.0f), point3f(0.0f,  0.0f, -1.0f), norm3f(0.0f, -1.0f,  0.0f)))
	};
	// Setup textures
	TextureSampler sampler;
	sampler.filterMin = filter;
	sampler.filterMag = filter;
	sampler.mipmapMode = TextureMipMapMode::None;
	sampler.wrapU = TextureWrap::ClampToEdge;
	sampler.wrapV = TextureWrap::ClampToEdge;
	sampler.wrapW = TextureWrap::ClampToEdge;
	sampler.anisotropy = 1.f;

	pass.material->set("u_equirectangularMap", sampler);
	pass.material->set("u_equirectangularMap", equirectangular);
	pass.material->set("CameraUniformBuffer", uniforms);
	pass.viewport = Rect{ 0, 0, width, height };

	// Convert
	for (int i = 0; i < 6; ++i)
	{
		camera.view = captureViews[i];
		uniforms->upload(&camera);
		pass.framebuffer->set(AttachmentType::Color0, cubemap, AttachmentFlag::None, i);
		pass.execute();
	}
	return cubemap;
}

};