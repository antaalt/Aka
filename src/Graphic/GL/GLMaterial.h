#if defined(AKA_USE_OPENGL)
#pragma once

#include <Aka/Graphic/Material.h>

namespace aka {

class GLMaterial : public Material
{
public:
	GLMaterial(Program::Ptr shader);
	~GLMaterial();

	void use() const;
};

};

#endif