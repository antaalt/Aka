#if defined(AKA_USE_D3D11)
#include "D3D11Texture.h"

#include "D3D11Context.h"
#include "D3D11Device.h"

#include <Aka/OS/Logger.h>

namespace aka {

DXGI_FORMAT d3dShaderDataFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_D16_UNORM:
		return DXGI_FORMAT_R16_TYPELESS;
	case DXGI_FORMAT_D32_FLOAT:
		return DXGI_FORMAT_R32_TYPELESS;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return DXGI_FORMAT_R24G8_TYPELESS;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	}
	return format;
}

DXGI_FORMAT d3dViewFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_D16_UNORM:
		return DXGI_FORMAT_R16_UNORM;
	case DXGI_FORMAT_D32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	}
	return format;
}

DXGI_FORMAT d3dFormat(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::R8:
	case TextureFormat::R8U: return DXGI_FORMAT_R8_UNORM;
	case TextureFormat::R16:
	case TextureFormat::R16U: return DXGI_FORMAT_R16_UNORM;
	case TextureFormat::R16F: return DXGI_FORMAT_R16_FLOAT;
	case TextureFormat::R32F: return DXGI_FORMAT_R32_FLOAT;

	case TextureFormat::RG8:
	case TextureFormat::RG8U: return DXGI_FORMAT_R8G8_UNORM;
	case TextureFormat::RG16:
	case TextureFormat::RG16U: return DXGI_FORMAT_R16G16_UNORM;
	case TextureFormat::RG16F: return DXGI_FORMAT_R16G16_FLOAT;
	case TextureFormat::RG32F: return DXGI_FORMAT_R32G32_FLOAT;

	case TextureFormat::RGB8:
	case TextureFormat::RGB8U: break; //return DXGI_FORMAT_R8G8B8_UNORM;
	case TextureFormat::RGB16:
	case TextureFormat::RGB16U: break; //return DXGI_FORMAT_R16G16B16_UNORM;
	case TextureFormat::RGB16F: break; //return DXGI_FORMAT_R16G16B16_FLOAT;
	case TextureFormat::RGB32F: return DXGI_FORMAT_R32G32B32_FLOAT;

	case TextureFormat::RGBA8:
	case TextureFormat::RGBA8U: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TextureFormat::RGBA16:
	case TextureFormat::RGBA16U: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TextureFormat::RGBA16F: return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case TextureFormat::RGBA32F: return DXGI_FORMAT_R32G32B32A32_FLOAT;

	case TextureFormat::Depth: return DXGI_FORMAT_D32_FLOAT;
	case TextureFormat::Depth16: return DXGI_FORMAT_D16_UNORM;
	case TextureFormat::Depth24: break; //return DXGI_FORMAT_D24_UNORM;
	case TextureFormat::Depth32: break; //return DXGI_FORMAT_D32_UNORM;
	case TextureFormat::Depth32F: return DXGI_FORMAT_D32_FLOAT;
	case TextureFormat::DepthStencil: return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case TextureFormat::Depth24Stencil8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case TextureFormat::Depth32FStencil8: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	default: break;
	}
	Logger::error("Format not supported : ", (int)format);
	return DXGI_FORMAT_UNKNOWN;
}
uint32_t d3dComponent(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::R8:
	case TextureFormat::R16:
	case TextureFormat::R8U:
	case TextureFormat::R16U:
	case TextureFormat::R16F:
	case TextureFormat::R32F:
		return 1;

	case TextureFormat::RG8:
	case TextureFormat::RG16:
	case TextureFormat::RG8U:
	case TextureFormat::RG16U:
	case TextureFormat::RG16F:
	case TextureFormat::RG32F:
		return 2;

	case TextureFormat::RGB8:
	case TextureFormat::RGB16:
	case TextureFormat::RGB8U:
	case TextureFormat::RGB16U:
	case TextureFormat::RGB16F:
	case TextureFormat::RGB32F:
		return 3;

	case TextureFormat::RGBA8:
	case TextureFormat::RGBA16:
	case TextureFormat::RGBA8U:
	case TextureFormat::RGBA16U:
	case TextureFormat::RGBA16F:
	case TextureFormat::RGBA32F:
		return 4;

	case TextureFormat::Depth:
	case TextureFormat::Depth16:
	case TextureFormat::Depth24:
	case TextureFormat::Depth32:
	case TextureFormat::Depth32F:
	case TextureFormat::DepthStencil:
	case TextureFormat::Depth0Stencil8:
	case TextureFormat::Depth24Stencil8:
	case TextureFormat::Depth32FStencil8:
		return 1;
	default:
		Logger::error("Format not supported : ", (int)format);
		return 0;
	}
}

D3D11Texture::D3D11Texture() :
	m_texture(nullptr),
	m_view(nullptr),
	m_component(0),
	m_d3dFormat(DXGI_FORMAT_UNKNOWN)
{
}

D3D11Texture::~D3D11Texture()
{
	if (m_view)
		m_view->Release();
	if (m_texture)
		m_texture->Release();
}

D3D11Texture* D3D11Texture::convert(const Texture::Ptr& texture)
{
	switch (texture->type())
	{
	case TextureType::Texture2D:
		return reinterpret_cast<D3D11Texture2D*>(texture.get());
	case TextureType::TextureCubeMap:
		return reinterpret_cast<D3D11TextureCubeMap*>(texture.get());
	case TextureType::Texture2DMultisample:
		return reinterpret_cast<D3D11Texture2DMultisample*>(texture.get());
	default:
		return nullptr;
	}
}

D3D11Texture2D::D3D11Texture2D(D3D11Device* device, uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* data) :
	Texture2D(width, height, format, flags),
	D3D11Texture(),
	m_device(device)
{
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.BindFlags = 0;

	bool isShaderResource = (TextureFlag::ShaderResource & flags) == TextureFlag::ShaderResource;
	bool isRenderTarget = (TextureFlag::RenderTarget & flags) == TextureFlag::RenderTarget;
	bool hasMips = (TextureFlag::GenerateMips & flags) == TextureFlag::RenderTarget;

	if (hasMips)
	{
		desc.MipLevels = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}
	if (isShaderResource)
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	m_d3dFormat = d3dFormat(format);
	m_component = d3dComponent(format);

	if (isRenderTarget)
	{
		if (isDepth(format))
			desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		else
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	}
	if (isShaderResource)
		desc.Format = d3dShaderDataFormat(m_d3dFormat);
	else
		desc.Format = m_d3dFormat;

	D3D11_SUBRESOURCE_DATA subResources{};
	subResources.pSysMem = data;
	subResources.SysMemPitch = width * size(format);
	subResources.SysMemSlicePitch = 0;
	D3D11_SUBRESOURCE_DATA* sub = nullptr;
	if (data != nullptr)
		sub = &subResources;
	D3D_CHECK_RESULT(m_device->device()->CreateTexture2D(&desc, sub, &m_texture));
	if (isShaderResource)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
		viewDesc.Format = d3dViewFormat(m_d3dFormat);
		viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipLevels = desc.MipLevels;
		viewDesc.Texture2D.MostDetailedMip = 0;

		D3D_CHECK_RESULT(m_device->device()->CreateShaderResourceView(m_texture, &viewDesc, &m_view));
		if (hasMips)
			m_device->context()->GenerateMips(m_view);
	}
	else
	{
		if (hasMips)
			Logger::warn("Trying to generate mips but texture is not a shader resource.");
	}
}
D3D11Texture2D::~D3D11Texture2D()
{
}
void D3D11Texture2D::upload(const void* data, uint32_t level)
{
	D3D11_BOX box{};
	box.left = 0;
	box.right = m_width;
	box.top = 0;
	box.bottom = m_height;
	box.front = 0;
	box.back = 1;
	m_device->context()->UpdateSubresource(
		m_texture,
		level,
		&box,
		data,
		m_width * m_component,
		0
	);
}
void D3D11Texture2D::upload(const Rect& rect, const void* data, uint32_t level)
{
	D3D11_BOX box{};
	box.left = (UINT)rect.x;
	box.right = (UINT)(rect.x + rect.w);
	box.top = (UINT)rect.y;
	box.bottom = (UINT)(rect.y + rect.h);
	box.front = 0;
	box.back = 1;
	m_device->context()->UpdateSubresource(
		m_texture,
		level,
		&box,
		data,
		m_width * m_component,
		0
	);
}
void D3D11Texture2D::download(void* data, uint32_t level)
{
	/*D3D11_BOX box{};
		box.left = 0;
		box.right = m_width;
		box.top = 0;
		box.bottom = m_height;
		box.front = 0;
		box.back = 1;

		// create staging texture
		if (!m_staging)
		{
			D3D11_TEXTURE2D_DESC desc;
			desc.Width = m_width;
			desc.Height = m_height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = m_d3dFormat;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.MiscFlags = 0;

			D3D_CHECK_RESULT(m_device->device()->CreateTexture2D(&desc, nullptr, &m_staging));
		}
		m_device->context()->CopySubresourceRegion(
			m_staging, level,
			0, 0, 0,
			m_texture, 0,
			&box
		);
		D3D11_MAPPED_SUBRESOURCE map{};
		D3D_CHECK_RESULT(m_device->context()->Map(m_staging, 0, D3D11_MAP_READ, 0, &map));
		memcpy(data, map.pData, m_width * m_height * 4);
		m_device->context()->Unmap(m_staging, 0);*/
}
TextureHandle D3D11Texture2D::handle() const
{
	return TextureHandle((uintptr_t)m_view);
}
void D3D11Texture2D::generateMips()
{
	if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
		return;
	m_device->context()->GenerateMips(m_view);
	m_flags = m_flags | TextureFlag::GenerateMips;
}

D3D11TextureCubeMap::D3D11TextureCubeMap(
	D3D11Device* device,
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	const void* px, const void* nx,
	const void* py, const void* ny,
	const void* pz, const void* nz
) :
	TextureCubeMap(width, height, format, flags),
	D3D11Texture(),
	m_device(device)
{
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	desc.BindFlags = 0;

	bool isShaderResource = (TextureFlag::ShaderResource & flags) == TextureFlag::ShaderResource;
	bool isRenderTarget = (TextureFlag::RenderTarget & flags) == TextureFlag::RenderTarget;
	bool hasMips = (TextureFlag::GenerateMips & flags) == TextureFlag::RenderTarget;

	if (hasMips)
	{
		desc.MipLevels = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	if (isShaderResource)
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	m_d3dFormat = d3dFormat(format);
	m_component = d3dComponent(format);

	if (isRenderTarget)
	{
		if (isDepth(format))
			desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		else
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	}
	if (isShaderResource)
		desc.Format = d3dShaderDataFormat(m_d3dFormat);
	else
		desc.Format = m_d3dFormat;

	D3D11_SUBRESOURCE_DATA data[6];
	const void* datas[6] = { px, nx, py, ny, pz, nz };
	for (size_t i = 0; i < 6; i++)
	{
		data[i].pSysMem = datas[i];
		data[i].SysMemPitch = width * size(format);
		data[i].SysMemSlicePitch = 0;
	}
	D3D11_SUBRESOURCE_DATA* pData = nullptr;
	if (px != nullptr)
		pData = data;
	D3D_CHECK_RESULT(m_device->device()->CreateTexture2D(&desc, pData, &m_texture));
	if (isShaderResource)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
		viewDesc.Format = d3dViewFormat(m_d3dFormat);
		viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBE;
		viewDesc.TextureCube.MipLevels = desc.MipLevels;
		viewDesc.TextureCube.MostDetailedMip = 0;

		D3D_CHECK_RESULT(m_device->device()->CreateShaderResourceView(m_texture, &viewDesc, &m_view));
		if (hasMips)
			m_device->context()->GenerateMips(m_view);
	}
	else
	{
		if (hasMips)
			Logger::warn("Trying to generate mips but texture is not a shader resource.");
	}
}
D3D11TextureCubeMap::~D3D11TextureCubeMap()
{
}
void D3D11TextureCubeMap::upload(const void* data, uint32_t layer, uint32_t level)
{
	D3D11_BOX box{};
	box.left = 0;
	box.right = m_width;
	box.top = 0;
	box.bottom = m_height;
	box.front = 0;
	box.back = 1;
	m_device->context()->UpdateSubresource(
		m_texture,
		D3D11CalcSubresource(level, layer, levels()),
		&box,
		data,
		m_width * m_component,
		0
	);
}
void D3D11TextureCubeMap::upload(const Rect& rect, const void* data, uint32_t layer, uint32_t level)
{
	D3D11_BOX box{};
	box.left = (UINT)rect.x;
	box.right = (UINT)(rect.x + rect.w);
	box.top = (UINT)rect.y;
	box.bottom = (UINT)(rect.y + rect.h);
	box.front = 0;
	box.back = 1;
	m_device->context()->UpdateSubresource(
		m_texture,
		D3D11CalcSubresource(level, layer, levels()),
		&box,
		data,
		m_width * m_component,
		0
	);
}
void D3D11TextureCubeMap::download(void* data, uint32_t layer, uint32_t level)
{
	/*D3D11_BOX box{};
	box.left = 0;
	box.right = m_width;
	box.top = 0;
	box.bottom = m_height;
	box.front = 0;
	box.back = 1;

	// create staging texture
	if (!m_staging)
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = m_width;
		desc.Height = m_height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = m_d3dFormat;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.MiscFlags = 0;

		D3D_CHECK_RESULT(m_device->device()->CreateTexture2D(&desc, nullptr, &m_staging));
	}
	m_device->context()->CopySubresourceRegion(
		m_staging, 0,
		0, 0, 0,
		m_texture, 0,
		&box
	);
	D3D11_MAPPED_SUBRESOURCE map{};
	D3D_CHECK_RESULT(m_device->context()->Map(m_staging, 0, D3D11_MAP_READ, 0, &map));
	memcpy(data, map.pData, m_width * m_height * 4);
	m_device->context()->Unmap(m_staging, 0);*/
}
TextureHandle D3D11TextureCubeMap::handle() const
{
	return TextureHandle((uintptr_t)m_view);
}
void D3D11TextureCubeMap::generateMips()
{
	if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
		return;
	m_device->context()->GenerateMips(m_view);
	m_flags = m_flags | TextureFlag::GenerateMips;
}

D3D11Texture2DMultisample::D3D11Texture2DMultisample(
	D3D11Device* device,
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	uint8_t samples,
	const void* data
) :
	Texture2DMultisample(width, height, format, flags),
	D3D11Texture(),
	m_device(device)
{
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = static_cast<UINT>(samples);
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.BindFlags = 0;

	bool isShaderResource = (TextureFlag::ShaderResource & flags) == TextureFlag::ShaderResource;
	bool isRenderTarget = (TextureFlag::RenderTarget & flags) == TextureFlag::RenderTarget;
	bool hasMips = (TextureFlag::GenerateMips & flags) == TextureFlag::RenderTarget;

	if (hasMips)
	{
		Logger::warn("Cannot generate mips for multisampled texture.");
		hasMips = false;
		flags = flags & ~TextureFlag::GenerateMips;
	}

	if (isShaderResource)
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_FORMAT_SUPPORT_MULTISAMPLE_LOAD;

	m_d3dFormat = d3dFormat(format);
	m_component = d3dComponent(format);

	if (isRenderTarget)
	{
		if (isDepth(format))
			desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL | D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET;
		else
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET | D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET;
	}
	if (isShaderResource)
		desc.Format = d3dShaderDataFormat(m_d3dFormat);
	else
		desc.Format = m_d3dFormat;

	D3D11_SUBRESOURCE_DATA subResources{};
	subResources.pSysMem = data;
	subResources.SysMemPitch = width * size(format);
	subResources.SysMemSlicePitch = 0;
	D3D11_SUBRESOURCE_DATA* sub = nullptr;
	if (data != nullptr)
		sub = &subResources;
	D3D_CHECK_RESULT(m_device->device()->CreateTexture2D(&desc, sub, &m_texture));
	if (isShaderResource)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
		viewDesc.Format = d3dViewFormat(m_d3dFormat);
		viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DMS;
		viewDesc.Texture2DMS.UnusedField_NothingToDefine;

		D3D_CHECK_RESULT(m_device->device()->CreateShaderResourceView(m_texture, &viewDesc, &m_view));
		if (hasMips)
			m_device->context()->GenerateMips(m_view);
	}
	else
	{
		if (hasMips)
			Logger::warn("Trying to generate mips but texture is not a shader resource.");
	}
}
D3D11Texture2DMultisample::~D3D11Texture2DMultisample()
{
}
void D3D11Texture2DMultisample::upload(const void* data)
{
	D3D11_BOX box{};
	box.left = 0;
	box.right = m_width;
	box.top = 0;
	box.bottom = m_height;
	box.front = 0;
	box.back = 1;
	m_device->context()->UpdateSubresource(
		m_texture,
		0,
		&box,
		data,
		m_width * m_component,
		0
	);
}
void D3D11Texture2DMultisample::upload(const Rect& rect, const void* data)
{
	D3D11_BOX box{};
	box.left = (UINT)rect.x;
	box.right = (UINT)(rect.x + rect.w);
	box.top = (UINT)rect.y;
	box.bottom = (UINT)(rect.y + rect.h);
	box.front = 0;
	box.back = 1;
	m_device->context()->UpdateSubresource(
		m_texture,
		0,
		&box,
		data,
		m_width * m_component,
		0
	);
}
void D3D11Texture2DMultisample::download(void* data)
{
	/*D3D11_BOX box{};
	box.left = 0;
	box.right = m_width;
	box.top = 0;
	box.bottom = m_height;
	box.front = 0;
	box.back = 1;

	// create staging texture
	if (!m_staging)
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = m_width;
		desc.Height = m_height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = m_d3dFormat;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.MiscFlags = 0;

		D3D_CHECK_RESULT(m_device->device()->CreateTexture2D(&desc, nullptr, &m_staging));
	}
	m_device->context()->CopySubresourceRegion(
		m_staging, 0,
		0, 0, 0,
		m_texture, 0,
		&box
	);
	D3D11_MAPPED_SUBRESOURCE map{};
	D3D_CHECK_RESULT(m_device->context()->Map(m_staging, 0, D3D11_MAP_READ, 0, &map));
	memcpy(data, map.pData, m_width * m_height * 4);
	m_device->context()->Unmap(m_staging, 0);*/
}
TextureHandle D3D11Texture2DMultisample::handle() const
{
	return TextureHandle((uintptr_t)m_view);
}
void D3D11Texture2DMultisample::generateMips()
{
	Logger::error("Cannot generate mips for multisampled texture.");
}

};

#endif