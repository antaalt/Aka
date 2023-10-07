#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Renderer/View.hpp>

namespace aka {

class Renderer;
class AssetLibrary;

class InstanceRenderer
{
public:
	InstanceRenderer(Renderer& _renderer);

	virtual void create() = 0;
	virtual void destroy() = 0;
	virtual void prepare(gfx::FrameHandle _frame) = 0;
	virtual void render(const View& view, gfx::FrameHandle _frame) = 0;
	virtual void resize(uint32_t _width, uint32_t _height) = 0;
	virtual void onReceive(const ShaderReloadedEvent& _event) = 0;
protected:
	gfx::GraphicDevice* getDevice();
	Renderer& getRenderer();
	AssetLibrary& getAssetLibrary();
private:
	Renderer& m_renderer;
};


};