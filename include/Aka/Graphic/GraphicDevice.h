#pragma once

#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Texture2D.h>
#include <Aka/Graphic/Texture2DMultisample.h>
#include <Aka/Graphic/TextureCubeMap.h>
#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Graphic/Backbuffer.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Graphic/RenderPass.h>
#include <Aka/Graphic/Device.h>
#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/Program.h>

namespace aka {

enum class GraphicAPI
{
	None,
	OpenGL3,
	DirectX11,
};

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
	uint32_t maxColorAttachments; // Max number of color attachments for framebuffer
	uint32_t maxElementIndices; // Max number of indices for index buffer
	uint32_t maxElementVertices; // Max number of vertices for vertex buffer
	uint32_t maxTextureUnits; // Max number of textures bound at same time
	uint32_t maxTextureSize; // Maximum size of a single texture
};

struct GraphicDeviceSettings
{
	GraphicAPI api; // Current API
	struct {
		uint32_t major; // Major version of the API
		uint32_t minor; // Minor version of the API
	} version;
	uint32_t profile; // Profile version of API shader
	GraphicCoordinates coordinates; // Specific api coordinates that might affect external code.
};

enum class GraphicFlag
{
	None,
	VerticalSynchronisation,
	HDRBackbuffer,
};

GraphicFlag operator|(const GraphicFlag& lhs, const GraphicFlag& rhs);
GraphicFlag operator&(const GraphicFlag& lhs, const GraphicFlag& rhs);
GraphicFlag operator~(const GraphicFlag& flag);

struct GraphicConfig
{
	GraphicFlag flags;
};

class GraphicDevice
{
public:
	GraphicDevice(const GraphicConfig& config);
	GraphicDevice(const GraphicDevice&) = delete;
	GraphicDevice& operator=(const GraphicDevice&) = delete;
	virtual ~GraphicDevice();
public:
	// Get the underlying api.
	const GraphicAPI& api() const;
	// Get device features
	const GraphicDeviceFeatures& features() const;
	// Get device flags
	GraphicFlag flags() const;
	// Get device settings
	const GraphicDeviceSettings& settings() const;
	// Get current api coordinates
	const GraphicCoordinates& coordinates() const;
	// Start a new frame
	void frame();
	// Present the frame
	void present();
	// Get the backbuffer
	Backbuffer::Ptr backbuffer();
public:
	// Render a render pass
	virtual void render(RenderPass& renderPass) = 0;
	// Dispatch a compute pass
	virtual void dispatch(ComputePass& computePass) = 0;
	// Copy the whole texture to destination texture
	virtual void copy(const Texture::Ptr& src, const Texture::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST) = 0;
	// Copy a texture
	virtual void blit(const Texture::Ptr& src, const Texture::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST, TextureFilter filter) = 0;

public:
	// Get the physical device info
	virtual Device getDevice(uint32_t id) = 0;
	// Get the physical device count
	virtual uint32_t getDeviceCount() = 0;
	// Create a 2D texture
	virtual Texture2D::Ptr createTexture2D(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		const void* data
	) = 0;
	// Create a 2D multisampled texture
	virtual Texture2DMultisample::Ptr createTexture2DMultisampled(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		uint8_t samples,
		const void* data
	) = 0;
	// Create a cubemap texture
	virtual TextureCubeMap::Ptr createTextureCubeMap(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		const void* px, const void* nx,
		const void* py, const void* ny,
		const void* pz, const void* nz
	) = 0;
	// Create a framebuffer
	virtual Framebuffer::Ptr createFramebuffer(Attachment* attachments, size_t count) = 0;
	// Create a buffer
	virtual Buffer::Ptr createBuffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data) = 0;
	// Create a mesh
	virtual Mesh::Ptr createMesh() = 0;
	// Compile a shader from content
	virtual Shader::Ptr compile(const char* content, ShaderType type) = 0;
	// Create a vertex program
	virtual Program::Ptr createVertexProgram(Shader::Ptr vert, Shader::Ptr frag, const VertexAttribute* attributes, size_t count) = 0;
	// Create a vertex program with geometry stage
	virtual Program::Ptr createGeometryProgram(Shader::Ptr vert, Shader::Ptr frag, Shader::Ptr geometry, const VertexAttribute* attributes, size_t count) = 0;
	// Create a compute program
	virtual Program::Ptr createComputeProgram(Shader::Ptr compute) = 0;
	// Create a material using given program.
	virtual Material::Ptr createMaterial(Program::Ptr shader) = 0;
protected:
	GraphicFlag m_flags;
	GraphicDeviceSettings m_settings;
	GraphicDeviceFeatures m_features; // Features of the device
	Backbuffer::Ptr m_backbuffer; // Backbuffer of the device
};

};