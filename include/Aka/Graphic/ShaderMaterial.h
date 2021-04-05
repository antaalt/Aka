#pragma once

#include "Shader.h"

namespace aka {

class ShaderMaterial
{
public:
	using Ptr = std::shared_ptr<ShaderMaterial>;
protected:
	ShaderMaterial(Shader::Ptr shader);
	ShaderMaterial(const ShaderMaterial&) = delete;
	const ShaderMaterial& operator=(const ShaderMaterial&) = delete;
	virtual ~ShaderMaterial();
public:
	static ShaderMaterial::Ptr create(Shader::Ptr shader);

	template <typename T>
	void set(const char* name, T value);
	template <typename T>
	void set(const char* name, const T* value, size_t size);

	Shader::Ptr getShader();

	UniformID getUniformID(const char* name);
	const Uniform* getUniform(const char* name) const;
	void setFloat1(const char* name, const float* value, size_t count);
	void setFloat2(const char* name, const float* value, size_t count);
	void setFloat3(const char* name, const float* value, size_t count);
	void setFloat4(const char* name, const float* value, size_t count);
	void setUint1(const char* name, const uint32_t* value, size_t count);
	void setUint2(const char* name, const uint32_t* value, size_t count);
	void setUint3(const char* name, const uint32_t* value, size_t count);
	void setUint4(const char* name, const uint32_t* value, size_t count);
	void setInt1(const char* name, const int32_t* value, size_t count);
	void setInt2(const char* name, const int32_t* value, size_t count);
	void setInt3(const char* name, const int32_t* value, size_t count);
	void setInt4(const char* name, const int32_t* value, size_t count);
	void setMatrix3(const char* name, const float* data, size_t count);
	void setMatrix4(const char* name, const float* data, size_t count);
	void setTexture(const char* name, const Texture::Ptr* texture, size_t count);
	void setImage(const char* name, const Texture::Ptr* texture, size_t count);

protected:
	float* findUniformOffset(const char* name);
protected:
	Shader::Ptr m_shader;
	std::vector<float> m_data;
	std::vector<Uniform> m_uniforms;
	std::vector<Texture::Ptr> m_textures;
	std::vector<Texture::Ptr> m_images;
};

}