#if defined(AKA_USE_OPENGL)
#pragma once

#include "GLContext.h"

#include <Aka/Graphic/Texture2D.h>
#include <Aka/Graphic/TextureCubeMap.h>
#include <Aka/Graphic/Texture2DMultisample.h>

namespace aka {

class GLTexture2D : public Texture2D
{
public:
	GLTexture2D(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* data);
	~GLTexture2D();
	void upload(const void* data, uint32_t level) override;
	void upload(const Rect& rect, const void* data, uint32_t level) override;
	void download(void* data, uint32_t level) override;
	TextureHandle handle() const override;
	void generateMips() override;
private:
	GLuint m_textureID;
};

class GLTextureCubeMap : public TextureCubeMap
{
public:
	GLTextureCubeMap(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		const void* px = nullptr, const void* nx = nullptr,
		const void* py = nullptr, const void* ny = nullptr,
		const void* pz = nullptr, const void* nz = nullptr
	);
	~GLTextureCubeMap();
	void upload(const void* data, uint32_t layer, uint32_t level) override;
	void upload(const Rect& rect, const void* data, uint32_t layer, uint32_t level) override;
	void download(void* data, uint32_t layer, uint32_t level) override;
	TextureHandle handle() const override;
	void generateMips() override;
private:
	GLuint m_textureID;
};


class GLTexture2DMultisample : public Texture2DMultisample
{
public:
	GLTexture2DMultisample(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		uint8_t samples,
		const void* data = nullptr
	);
	~GLTexture2DMultisample();
	void upload(const void* data) override;
	void upload(const Rect& rect, const void* data) override;
	void download(void* data) override;
	TextureHandle handle() const override;
	void generateMips() override;
private:
	GLuint m_textureID;
};


};

#endif