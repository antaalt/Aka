#if defined(AKA_USE_D3D11)
#pragma once

#include <Aka/Graphic/GraphicDevice.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace aka {

class D3D11Context;

class D3D11Device : public GraphicDevice
{
public:
	D3D11Device(const GraphicConfig& config);
	~D3D11Device();
public:
	D3D11Context* ctx();
	ID3D11Device* device();
	ID3D11DeviceContext* context();
public:
	void render(RenderPass& renderPass) override;
	void dispatch(ComputePass& computePass) override;
public:
	void copy(const Texture::Ptr& src, const Texture::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST) override;
	void blit(const Texture::Ptr& src, const Texture::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST, TextureFilter filter) override;
public:
	Device getDevice(uint32_t id) override;
	uint32_t getDeviceCount() override;
	Texture2D::Ptr createTexture2D(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		const void* data
	) override;
	Texture2DMultisample::Ptr createTexture2DMultisampled(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		uint8_t samples,
		const void* data
	) override;
	TextureCubeMap::Ptr createTextureCubeMap(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		const void* px, const void* nx,
		const void* py, const void* ny,
		const void* pz, const void* nz
	) override;
	Framebuffer::Ptr createFramebuffer(Attachment* attachments, size_t count) override;
	Buffer::Ptr createBuffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data) override;
	Mesh::Ptr createMesh() override;
	Shader::Ptr compile(const char* content, ShaderType type) override;
	Program::Ptr createVertexProgram(Shader::Ptr vert, Shader::Ptr frag, const VertexAttribute* attributes, size_t count) override;
	Program::Ptr createGeometryProgram(Shader::Ptr vert, Shader::Ptr frag, Shader::Ptr geometry, const VertexAttribute* attributes, size_t count) override;
	Program::Ptr createComputeProgram(Shader::Ptr compute);
	Material::Ptr createMaterial(Program::Ptr shader) override;
private:
	D3D11Context* m_context;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;
};

};

#endif