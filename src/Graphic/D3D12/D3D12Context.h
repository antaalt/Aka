#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Core/Config.h>
#include <Aka/OS/Logger.h>

#if defined(AKA_USE_D3D12)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#if defined(min)
#undef min
#endif
#if defined(max)
#undef max
#endif

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>


#define D3D12_CHECK_RESULT(result)			\
{											\
	HRESULT hr = (result);				    \
	if (FAILED(hr)) {				        \
		char buffer[256];					\
		snprintf(							\
			buffer,							\
			256,							\
			"%s (%s at %s:%d)",				\
			std::system_category().         \
				message(hr).c_str(),       \
			AKA_STRINGIFY(result),			\
			__FILE__,						\
			__LINE__						\
		);									\
		::aka::Logger::error(buffer);       \
		AKA_DEBUG_BREAK;                    \
	}										\
}

namespace aka {

struct VulkanContext
{
	void initialize(PlatformDevice* platform, const GraphicConfig& config);
	void shutdown();

	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter;
	Microsoft::WRL::ComPtr<ID3D12Device2> device;
};

};

#endif