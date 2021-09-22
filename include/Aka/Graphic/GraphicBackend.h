#pragma once

#include <Aka/Graphic/Texture2D.h>
#include <Aka/Graphic/Texture2DMultisample.h>
#include <Aka/Graphic/TextureCubeMap.h>
#include <Aka/Graphic/Device.h>
#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/Material.h>
#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/Mesh.h>
#include <Aka/Graphic/RenderPass.h>
#include <Aka/OS/FileSystem.h>

#if defined(AKA_USE_D3D11)
struct ID3D11Device;
struct ID3D11DeviceContext;
#endif

namespace aka {

enum class GraphicApi 
{
	None,
	OpenGL,
	DirectX11,
};

// Specific api coordinates that might affect external code.
struct GraphicCoordinates
{
	// Is origin of texture bottom left or top left.
	// Textures data must be flipped at loading if originTextureBottomLeft is true.
	// This need to be respected to be render target compatible.
	// D3D / Metal / Consoles origin is top left
	// OpenGL / OpenGL ES origin is bottom left
	bool originTextureBottomLeft; 

	// Is UV (0, 0) bottom left or top left (pixel space coordinate).
	// UV of models must be flipped if originUVBottomLeft is false.
	// This need to be respected to be render target compatible.
	bool originUVBottomLeft;

	// Is clip space between [0, 1] or [-1, 1]
	// This affect mostly projection matrix which are responsible of changing view space to clip space.
	bool clipSpacePositive;

	// Is render Y Axis up
	// This affect mostly projection matrix which are responsible of changing view space to clip space
	// Vulkan is false, others are true
	bool renderAxisYUp;

	// Graphics math conventions is up to the dev (left-handed vs right-handed, column-major vs row-major...)
};

struct GraphicDeviceFeatures
{
	GraphicApi api; // Current API
	struct 
	{
		uint32_t major; // Major version of the API
		uint32_t minor; // Minor version of the API
	} version;
	uint32_t profile; // Profile version of API shader

	uint32_t maxColorAttachments; // Max number of color attachments for framebuffer
	uint32_t maxElementIndices; // Max number of indices for index buffer
	uint32_t maxElementVertices; // Max number of vertices for vertex buffer
	uint32_t maxTextureUnits; // Max number of textures bound at same time
	uint32_t maxTextureSize; // Maximum size of a single texture

	GraphicCoordinates coordinates;
};

class GraphicBackend {
	GraphicBackend() {}
	GraphicBackend(GraphicBackend&) = delete;
	GraphicBackend& operator=(GraphicBackend&) = delete;
	~GraphicBackend() {}
public:
	// Get the current API
	static GraphicApi api();
	// Get the current API
	static const GraphicDeviceFeatures& features();
	// Initialize the graphic API
	static void initialize(uint32_t width, uint32_t height);
	// Destroy the graphic API
	static void destroy();
public:
	// Start a new frame
	static void frame();
	// Present the frame
	static void present();
	// Get the backbuffer
	static Framebuffer::Ptr backbuffer();
	// Render a render pass
	static void render(RenderPass& renderPass);
	// Dispatch a compute pass
	static void dispatch(ComputePass& computePass);
	// Take a screenshot
	static void screenshot(const Path& path); // TODO move to backbuffer ?
	// Set the vsync
	static void vsync(bool enabled);
public:
#if defined(AKA_USE_D3D11)
	// Get D3D11 device
	static ID3D11Device* getD3D11Device();
	// Get D3D11 device context
	static ID3D11DeviceContext* getD3D11DeviceContext();
#endif
protected:
	friend struct Device;
	static Device getDevice(uint32_t id);
	static uint32_t deviceCount();

	friend class Texture2D;
	static Texture2D::Ptr createTexture2D(
		uint32_t width, uint32_t height, 
		TextureFormat format, TextureFlag flags, 
		const void* data
	);
	friend class Texture2DMultisample;
	static Texture2DMultisample::Ptr createTexture2DMultisampled(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		uint8_t samples,
		const void* data
	);
	friend class TextureCubeMap;
	static TextureCubeMap::Ptr createTextureCubeMap(
		uint32_t width, uint32_t height, 
		TextureFormat format, TextureFlag flags,
		const void* px, const void* nx,
		const void* py, const void* ny,
		const void* pz, const void* nz
	);

	friend class Framebuffer;
	static Framebuffer::Ptr createFramebuffer(Attachment* attachments, size_t count);

	friend class Buffer;
	static Buffer::Ptr createBuffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data);

	friend class Mesh;
	static Mesh::Ptr createMesh();

	friend class Shader;
	static Shader::Ptr compile(const char* content, ShaderType type);

	friend class Program;
	static Program::Ptr createVertexProgram(Shader::Ptr vert, Shader::Ptr frag, const VertexAttribute* attributes, size_t count);
	static Program::Ptr createGeometryProgram(Shader::Ptr vert, Shader::Ptr frag, Shader::Ptr geometry, const VertexAttribute* attributes, size_t count);
	static Program::Ptr createComputeProgram(Shader::Ptr compute);

	friend class Material;
	static Material::Ptr createMaterial(Program::Ptr shader);
};

}