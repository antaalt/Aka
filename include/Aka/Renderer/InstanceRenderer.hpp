#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Renderer/View.hpp>
#include <Aka/Renderer/Instance.hpp>

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

	virtual InstanceHandle createInstance(AssetID assetID) = 0;
	virtual void updateInstanceTransform(InstanceHandle instanceHandle, const mat4f& transform) = 0;
	virtual void destroyInstance(InstanceHandle instanceHandle) = 0;
protected:
	gfx::GraphicDevice* getDevice();
	Renderer& getRenderer();
	AssetLibrary& getAssetLibrary();
private:
	Renderer& m_renderer;
};


};