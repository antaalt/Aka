#if defined(AKA_USE_D3D11)
#pragma once

#include <Aka/Graphic/Material.h>

namespace aka {

class D3D11Device;

class D3D11Material : public Material
{
public:
	D3D11Material(D3D11Device* device, Program::Ptr shader);
	~D3D11Material();

	void use() const;
private:
	D3D11Device* m_device;
};

};

#endif