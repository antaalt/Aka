#if defined(AKA_USE_D3D11)
#include "D3D11Device.h"

#include "D3D11Context.h"
#include "D3D11Backbuffer.h"
#include "D3D11Mesh.h"
#include "D3D11Material.h"
#include "D3D11Buffer.h"
#include "D3D11Texture.h"
#include "D3D11Framebuffer.h"
#include "D3D11Program.h"
#include "Internal/D3D11Blit.h"

#include <Aka/OS/Logger.h>

namespace aka {

D3D11Context* D3D11Device::ctx()
{
	return m_context;
}
ID3D11Device* D3D11Device::device()
{
	return m_device;
}
ID3D11DeviceContext* D3D11Device::context()
{
	return m_deviceContext;
}

D3D11Device::D3D11Device(uint32_t width, uint32_t height) :
	GraphicDevice(width, height),
	m_context(nullptr),
	m_device(nullptr),
	m_deviceContext(nullptr)
{
	bool vsync = true;
	bool fullscreen = false;
	Device device = getDevice(0);
	// --- SwapChain ---
	// Initialize the swap chain description.
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;
	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// Set the refresh rate of the back buffer.
	if (vsync) // default vsync
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = device.monitors[0].numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = device.monitors[0].denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}
	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = PlatformBackend::getWindowsWindowHandle();
	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	// Set to full screen or windowed mode.
	swapChainDesc.Windowed = !fullscreen;
	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;
	// Set the feature level to DirectX 11.
	D3D_FEATURE_LEVEL featureLevel;
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	IDXGISwapChain* swapchain;
	UINT flags = 0;
#if defined(DEBUG)
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_CHECK_RESULT(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags,
		&featureLevel, 1,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&swapchain,
		&m_device,
		nullptr,
		&m_deviceContext
	));
	m_backbuffer = std::make_shared<D3D11Backbuffer>(this, swapchain, width, height);

	// Check Features
	// We are using DirectX11 in this backend.
	m_features.api = GraphicApi::DirectX11;
	m_features.version.major = 11;
	m_features.version.minor = 0;
	m_features.profile = 50;
	// D3D11_COMMONSHADER_CONSTANT_BUFFER_HW_SLOT_COUNT 
	// D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT 
	m_features.maxTextureUnits = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
	m_features.maxTextureSize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	m_features.maxColorAttachments = 0; // ?
	m_features.maxElementIndices = 0; // ?
	m_features.maxElementVertices = 0; // ?
	m_features.coordinates.clipSpacePositive = true; // D3D11 clip space is [0, 1]
	m_features.coordinates.originTextureBottomLeft = false; // D3D11 start reading texture at top left.
	m_features.coordinates.originUVBottomLeft = false; // D3D11 UV origin is top left
	m_features.coordinates.renderAxisYUp = true; // D3D11 render axis y is up

	m_context = new D3D11Context(this);
}

D3D11Device::~D3D11Device()
{
	if (m_context)
		delete m_context;
	m_backbuffer.reset();
	if (m_deviceContext)
		m_deviceContext->Release();
	if (m_device)
		m_device->Release();
}

void D3D11Device::render(RenderPass& pass)
{
	{
		// Unbind resources to avoid warning & D3D unbinding texture that is also set as rendertarget
		ID3D11RenderTargetView* nullRenderTarget[5] = { nullptr };
		ID3D11DepthStencilState* nullDepthStencil = nullptr;
		ID3D11ShaderResourceView* const pSRV[10] = { nullptr };
		ID3D11SamplerState* const pSamplers[10] = { nullptr };
		m_deviceContext->OMSetRenderTargets(5, nullRenderTarget, nullptr);
		m_deviceContext->PSSetShaderResources(0, 10, pSRV);
		m_deviceContext->PSSetSamplers(0, 10, pSamplers);
		m_deviceContext->VSSetShaderResources(0, 10, pSRV);
		m_deviceContext->VSSetSamplers(0, 10, pSamplers);
		m_deviceContext->CSSetShaderResources(0, 10, pSRV);
		m_deviceContext->CSSetSamplers(0, 10, pSamplers);
		m_deviceContext->GSSetShaderResources(0, 10, pSRV);
		m_deviceContext->GSSetSamplers(0, 10, pSamplers);
	}
	{
		// Shader
		if (pass.material == nullptr)
		{
			Logger::error("No material set for render pass");
			return;
		}
		else
		{
			D3D11Material* d3dMaterial = (D3D11Material*)pass.material.get();
			d3dMaterial->use();
		}
	}

	{
		// Rasterizer
		ID3D11RasterizerState* rasterState = m_context->getRasterizerState(pass.cull);
		if (rasterState != nullptr)
			m_deviceContext->RSSetState(rasterState);
		else
			m_deviceContext->RSSetState(nullptr);
	}

	{
		// Viewport
		if (pass.viewport.w == 0 || pass.viewport.h == 0)
		{
			D3D11_VIEWPORT viewport;
			viewport.Width = static_cast<float>(pass.framebuffer->width());
			viewport.Height = static_cast<float>(pass.framebuffer->height());
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			viewport.TopLeftX = 0.f;
			viewport.TopLeftY = 0.f;

			// Set the viewport.
			m_deviceContext->RSSetViewports(1, &viewport);
		}
		else
		{
			D3D11_VIEWPORT viewport;
			viewport.Width = (float)pass.viewport.w;
			viewport.Height = (float)pass.viewport.h;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			viewport.TopLeftX = (float)pass.viewport.x;
			viewport.TopLeftY = (float)pass.viewport.y;

			// Set the viewport.
			m_deviceContext->RSSetViewports(1, &viewport);
		}
	}

	{
		// Scissor
		if (pass.scissor.w > 0 && pass.scissor.h > 0)
		{
			D3D11_RECT scissor;
			scissor.right = (LONG)pass.scissor.x + pass.scissor.w;
			scissor.bottom = (LONG)pass.scissor.y + pass.scissor.h;
			scissor.left = (LONG)pass.scissor.x;
			scissor.top = (LONG)pass.scissor.y;

			// Set the scissor.
			m_deviceContext->RSSetScissorRects(1, &scissor);
		}
	}

	{
		// Set Framebuffer
		if (pass.framebuffer == m_backbuffer)
		{
			D3D11Backbuffer* backbuffer = (D3D11Backbuffer*)m_backbuffer.get();
			ID3D11RenderTargetView* view = backbuffer->m_renderTargetView;
			m_deviceContext->OMSetRenderTargets(1, &view, backbuffer->m_depthStencilView);
		}
		else
		{
			D3D11Framebuffer* framebuffer = (D3D11Framebuffer*)pass.framebuffer.get();
			std::vector<ID3D11RenderTargetView*> views;
			for (size_t i = 0; i < framebuffer->m_colorViews.size(); i++)
			{
				auto it = framebuffer->m_colorViews.find((AttachmentType)((int)AttachmentType::Color0 + i));
				if (it != framebuffer->m_colorViews.end())
					views.push_back(it->second);
				else
					views.push_back(nullptr);
			}
			if (views.size() != 0)
				m_deviceContext->OMSetRenderTargets((UINT)views.size(), views.data(), framebuffer->m_depthStencilView);
			else
				m_deviceContext->OMSetRenderTargets(0, nullptr, framebuffer->m_depthStencilView);
		}
	}

	{
		if (pass.clear.mask != ClearMask::None)
			pass.framebuffer->clear(pass.clear.color, pass.clear.depth, pass.clear.stencil, pass.clear.mask);
	}

	{
		// Depth
		ID3D11DepthStencilState* depthState = m_context->getDepthState(pass.depth, pass.stencil);
		if (depthState != nullptr)
			m_deviceContext->OMSetDepthStencilState(depthState, 1);
		else
			m_deviceContext->OMSetDepthStencilState(nullptr, 1);

	}

	{
		// Blend
		ID3D11BlendState* blendState = m_context->getBlendingState(pass.blend);
		if (blendState != nullptr)
		{
			float blendFactor[4] = { 1.f, 1.f, 1.f, 1.f };
			m_deviceContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);
		}
		else
		{
			m_deviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		}
	}

	{
		// Mesh
		if (pass.submesh.mesh == nullptr)
		{
			Logger::error("No mesh set for render pass");
			return;
		}
		else
		{
			if (pass.submesh.mesh->isIndexed())
				pass.submesh.drawIndexed();
			else
				pass.submesh.draw();
		}
	}
}

void D3D11Device::dispatch(ComputePass& pass)
{
	throw std::runtime_error("Not implemented");
}

void D3D11Device::copy(const Texture::Ptr& src, const Texture::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST) 
{
	AKA_ASSERT(src->format() == dst->format(), "Invalid format");
	AKA_ASSERT(regionSRC.x + regionSRC.width <= src->width() && regionSRC.y + regionSRC.height <= src->height(), "Region not in range.");
	AKA_ASSERT(regionDST.x + regionDST.width <= dst->width() && regionDST.y + regionDST.height <= dst->height(), "Region not in range.");
	AKA_ASSERT(regionSRC.width == regionDST.width && regionSRC.height == regionDST.height, "Region size invalid.");

	D3D11_BOX box{};
	box.left = regionSRC.x;
	box.right = regionSRC.x + regionSRC.width;
	box.top = regionSRC.y;
	box.bottom = regionSRC.y + regionSRC.height;
	box.front = 0;
	box.back = 1;

	D3D11_BOX* pBox = nullptr;
	if (regionSRC.width != regionDST.width || regionSRC.height != regionDST.height)
		pBox = &box;

	D3D11Texture* srcTexture = D3D11Texture::convert(src);
	D3D11Texture* dstTexture = D3D11Texture::convert(dst);
	m_deviceContext->CopySubresourceRegion(
		dstTexture->m_texture, regionDST.level,
		regionDST.x, regionDST.y, 0,
		srcTexture->m_texture, regionSRC.level,
		pBox
	);
	if ((TextureFlag::GenerateMips & dst->flags()) == TextureFlag::GenerateMips)
		dst->generateMips();
}

void D3D11Device::blit(const Texture::Ptr& src, const Texture::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST, TextureFilter filter) 
{
	if (src->format() == dst->format() && regionSRC.width == regionDST.width && regionSRC.height == regionDST.height)
	{
		copy(src, dst, regionSRC, regionDST);
	}
	else
	{
		if (isDepthStencil(src->format()))
		{
			AKA_ASSERT(isDepthStencil(src->format()), "Invalid format");
			AKA_ASSERT(regionSRC.x + regionSRC.width <= src->width() && regionSRC.y + regionSRC.height <= src->height(), "Region not in range.");
			AKA_ASSERT(regionDST.x + regionDST.width <= dst->width() && regionDST.y + regionDST.height <= dst->height(), "Region not in range.");
			AKA_ASSERT(regionSRC.width == regionDST.width && regionSRC.height == regionDST.height, "Region size invalid.");

			D3D11_BOX box{};
			box.left = regionSRC.x;
			box.right = regionSRC.x + regionSRC.width;
			box.top = regionSRC.y;
			box.bottom = regionSRC.y + regionSRC.height;
			box.front = 0;
			box.back = 1;

			D3D11_BOX* pBox = nullptr;
			if (regionSRC.width != regionDST.width || regionSRC.height != regionDST.height)
				pBox = &box;

			D3D11Texture* srcTexture = D3D11Texture::convert(src);
			D3D11Texture* dstTexture = D3D11Texture::convert(dst);
			m_deviceContext->CopySubresourceRegion(
				dstTexture->m_texture, regionDST.level,
				regionDST.x, regionDST.y, 0,
				srcTexture->m_texture, regionSRC.level,
				pBox
			);
		}
		else if (isDepth(src->format()))
		{
			AKA_ASSERT(isDepth(dst->format()), "");
			Attachment attachment = Attachment{ AttachmentType::Depth, dst, AttachmentFlag::None, regionDST.layer, regionDST.level };
			Framebuffer::Ptr framebuffer = Framebuffer::create(&attachment, 1);
			blitDepth(src, framebuffer, regionSRC, regionDST, filter);
		}
		else
		{
			Attachment attachment = Attachment{ AttachmentType::Color0, dst, AttachmentFlag::None, regionDST.layer, regionDST.level };
			Framebuffer::Ptr framebuffer = Framebuffer::create(&attachment, 1);
			blitColor(src, framebuffer, regionSRC, regionDST, filter);
		}
	}
}

Device D3D11Device::getDevice(uint32_t id)
{
	Device device{};

	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput; // monitor
	// Create a DirectX graphics interface factory.
	D3D_CHECK_RESULT(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory));
	// Use the factory to create an adapter for the primary graphics interface (video card).
	D3D_CHECK_RESULT(factory->EnumAdapters(id, &adapter));
	// Enumerate the primary adapter output (monitor).
	D3D_CHECK_RESULT(adapter->EnumOutputs(id, &adapterOutput));

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	uint32_t numModes = 0;
	D3D_CHECK_RESULT(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr));

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	std::vector<DXGI_MODE_DESC> displayModeList(numModes);

	// Now fill the display mode list structures.
	D3D_CHECK_RESULT(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList.data()));

	// Now go through all the display modes
	device.monitors.resize(numModes);
	for (unsigned int i = 0; i < numModes; i++)
	{
		Monitor& monitor = device.monitors[i];
		monitor.width = displayModeList[i].Width;
		monitor.height = displayModeList[i].Height;
		monitor.numerator = displayModeList[i].RefreshRate.Numerator;
		monitor.denominator = displayModeList[i].RefreshRate.Denominator;
	}
	// Get the adapter (video card) description.
	DXGI_ADAPTER_DESC adapterDesc;
	D3D_CHECK_RESULT(adapter->GetDesc(&adapterDesc));

	// Store the dedicated video card memory in megabytes.
	device.memory = (uint32_t)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	size_t stringLength;
	int error = wcstombs_s(&stringLength, device.vendor, 128, adapterDesc.Description, 128);
	if (error != 0)
		Logger::error("Could not get video card name");

	// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = 0;

	// Release the adapter.
	adapter->Release();
	adapter = 0;

	// Release the factory.
	factory->Release();
	factory = 0;

	return device;
}

uint32_t D3D11Device::getDeviceCount()
{
	return 0;
}

Texture2D::Ptr D3D11Device::createTexture2D(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	const void* data
)
{
	return std::make_shared<D3D11Texture2D>(this, width, height, format, flags, data);
}

Texture2DMultisample::Ptr D3D11Device::createTexture2DMultisampled(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	uint8_t samples,
	const void* data
)
{
	return std::make_shared<D3D11Texture2DMultisample>(this, width, height, format, flags, samples, data);
}

TextureCubeMap::Ptr D3D11Device::createTextureCubeMap(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	const void* px, const void* nx,
	const void* py, const void* ny,
	const void* pz, const void* nz
)
{
	return std::make_shared<D3D11TextureCubeMap>(this, width, height, format, flags, px, nx, py, ny, pz, nz);
}

Framebuffer::Ptr D3D11Device::createFramebuffer(Attachment* attachments, size_t count)
{
	return std::make_shared<D3D11Framebuffer>(this, attachments, count);
}

Buffer::Ptr D3D11Device::createBuffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return std::make_shared<D3D11Buffer>(this, type, size, usage, access, data);
}

Mesh::Ptr D3D11Device::createMesh()
{
	return std::make_shared<D3D11Mesh>(this);
}

Shader::Ptr D3D11Device::compile(const char* content, ShaderType type)
{
	return D3D11Shader::compileHLSL(content, type);
}

Program::Ptr D3D11Device::createVertexProgram(Shader::Ptr vert, Shader::Ptr frag, const VertexAttribute* attributes, size_t count)
{
	return std::make_shared<D3D11Program>(this, vert, frag, nullptr, nullptr, attributes, count);
}
Program::Ptr D3D11Device::createGeometryProgram(Shader::Ptr vert, Shader::Ptr frag, Shader::Ptr geometry, const VertexAttribute* attributes, size_t count)
{
	return std::make_shared<D3D11Program>(this, vert, frag, geometry, nullptr, attributes, count);
}
Program::Ptr D3D11Device::createComputeProgram(Shader::Ptr compute)
{
	VertexAttribute dummy{};
	return std::make_shared<D3D11Program>(this, nullptr, nullptr, nullptr, compute, &dummy, 0);
}
Material::Ptr D3D11Device::createMaterial(Program::Ptr shader)
{
	return std::make_shared<D3D11Material>(this, shader);
}

};
#endif