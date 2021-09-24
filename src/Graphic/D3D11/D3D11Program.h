#if defined(AKA_USE_D3D11)
#pragma once

#include "D3D11Context.h"

#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Shader.h>

namespace aka {

class D3D11Shader : public Shader
{
	friend class D3D11Device;
	friend class D3D11Program;
public:
	D3D11Shader(ID3D10Blob* shaderBuffer);
	~D3D11Shader();

	static Shader::Ptr compileHLSL(const char* content, ShaderType type);
private:
	ID3D10Blob* m_shaderBuffer;
};

class D3D11Program : public Program
{
	friend class D3D11Device;
	friend class D3D11Material;
public:
	D3D11Program(D3D11Device* device, Shader::Ptr vertex, Shader::Ptr fragment, Shader::Ptr geometry, Shader::Ptr compute, const VertexAttribute* attributes, size_t count);
	~D3D11Program();

	void use();
private:
	D3D11Device* m_device;
	ID3D11InputLayout* m_layout;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11GeometryShader* m_geometryShader;
	ID3D11ComputeShader* m_computeShader;
};


};

#endif