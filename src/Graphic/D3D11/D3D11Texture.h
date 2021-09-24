#if defined(AKA_USE_D3D11)
#pragma once

#include "D3D11Context.h"

#include <Aka/Graphic/Texture2D.h>
#include <Aka/Graphic/TextureCubeMap.h>
#include <Aka/Graphic/Texture2DMultisample.h>

namespace aka {

class D3D11Device;

struct D3D11Texture
{
	friend D3D11Framebuffer;
	friend D3D11Backbuffer;
	friend D3D11Device;

	D3D11Texture();
	virtual ~D3D11Texture();

	// Convert texture to D3D11 texture
	static D3D11Texture* convert(const Texture::Ptr& texture);

protected:
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_view;
	DXGI_FORMAT m_d3dFormat;
	uint32_t m_component;
};

class D3D11Texture2D : public Texture2D, public D3D11Texture
{
public:
	D3D11Texture2D(D3D11Device* device, uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* data);
	~D3D11Texture2D();
	void upload(const void* data, uint32_t level) override;
	void upload(const Rect& rect, const void* data, uint32_t level) override;
	void download(void* data, uint32_t level) override;
	TextureHandle handle() const override;
	void generateMips() override;
private:
	D3D11Device* m_device;
};

class D3D11TextureCubeMap : public TextureCubeMap, public D3D11Texture
{
public:
	D3D11TextureCubeMap(
		D3D11Device* device,
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		const void* px = nullptr, const void* nx = nullptr,
		const void* py = nullptr, const void* ny = nullptr,
		const void* pz = nullptr, const void* nz = nullptr
	);
	~D3D11TextureCubeMap();
	void upload(const void* data, uint32_t layer, uint32_t level) override;
	void upload(const Rect& rect, const void* data, uint32_t layer, uint32_t level) override;
	void download(void* data, uint32_t layer, uint32_t level) override;
	TextureHandle handle() const override;
	void generateMips() override;
private:
	D3D11Device* m_device;
};


class D3D11Texture2DMultisample : public Texture2DMultisample, public D3D11Texture
{
public:
	D3D11Texture2DMultisample(
		D3D11Device* device,
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		uint8_t samples,
		const void* data = nullptr
	);
	~D3D11Texture2DMultisample();
	void upload(const void* data) override;
	void upload(const Rect& rect, const void* data) override;
	void download(void* data) override;
	TextureHandle handle() const override;
	void generateMips() override;
private:
	D3D11Device* m_device;
};


};

#endif