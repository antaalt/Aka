#pragma once
#if defined(AKA_USE_D3D11)
#include <Aka/Core/Debug.h>
#include <Aka/Platform/Platform.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <stdexcept>
#include <array>
#include <map>
#include <system_error>

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

#define D3D_CHECK_RESULT(result)    \
{                                   \
    HRESULT res = (result);         \
    if (FAILED(res)) {              \
        char buffer[256];           \
        snprintf(                   \
            buffer,                 \
            256,                    \
            "%s (%s at %s:%d)",     \
            std::system_category(). \
			message(res).c_str(),   \
            AKA_STRINGIFY(result),  \
            __FILE__,               \
            __LINE__				\
        );							\
        ::aka::Logger::error(buffer);\
		AKA_DEBUG_BREAK;            \
	}								\
}

namespace aka {

class D3D11Device;
class D3D11Framebuffer;
class D3D11Backbuffer;

class D3D11Texture2D;
class D3D11TextureCubeMap;
class D3D11Texture2DMultisample;

class D3D11Buffer;

class D3D11Shader;
class D3D11Program;
class D3D11Material;

class D3D11Mesh;

class D3D11Context
{
public: // States
	D3D11Context(D3D11Device* device);
	~D3D11Context();

	void log();
	ID3D11BlendState* getBlendingState(const Blending& blending);
	ID3D11DepthStencilState* getDepthState(const Depth& depth, const Stencil& stencil);
	ID3D11RasterizerState* getRasterizerState(const Culling& culling);
	ID3D11SamplerState* getSamplerState(const TextureSampler& sampler);
private:
	D3D11Device* m_device;
	ID3D11InfoQueue* m_debugInfoQueue;
private: // Cache
	struct D3D11Blend {
		Blending blend = Blending::none;
		ID3D11BlendState* blendState = nullptr;
	};
	struct D3D11Depth {
		Depth depth = Depth::none;
		Stencil stencil = Stencil::none;
		ID3D11DepthStencilState* depthState = nullptr;
	};
	struct D3D11RasterPass {
		Culling cull{};
		ID3D11RasterizerState* rasterState = nullptr;
	};
	struct D3D11Sampler {
		TextureSampler sampler = {};
		ID3D11SamplerState* samplerState = nullptr;
	};
	std::vector<D3D11Blend> m_blendCache;
	std::vector<D3D11Depth> m_depthCache;
	std::vector<D3D11RasterPass> m_rasterCache;
	std::vector<D3D11Sampler> m_samplerCache;
};

};

#endif