#include <Aka/Graphic/Program.h>

#include <Aka/Core/Application.h>

namespace aka {
namespace gfx {

ShaderBindingState ShaderBindingState::merge(const ShaderBindingState& lhs, const ShaderBindingState& rhs)
{
	ShaderBindingState bindings = lhs;
	for (uint32_t i = 0; i < rhs.count; i++)
	{
		if (rhs.bindings[i].type != ShaderBindingType::None)
		{
			if (lhs.bindings[i].type == ShaderBindingType::None)
			{
				bindings.bindings[i] = rhs.bindings[i];
				bindings.count = max(bindings.count, i + 1);
			}
			else
			{
				AKA_ASSERT(rhs.bindings[i].type == lhs.bindings[i].type, "Mismatching bindings");
				AKA_ASSERT(rhs.bindings[i].count == lhs.bindings[i].count, "Mismatching count");
				bindings.bindings[i].shaderType = bindings.bindings[i].shaderType | rhs.bindings[i].shaderType;
			}
		}
	}
	return bindings;
}

bool Program::hasVertexStage() const
{
	return vertex != nullptr;
}
bool Program::hasFragmentStage() const
{
	return fragment != nullptr;
}
bool Program::hasGeometryStage() const
{
	return geometry != nullptr;
}
bool Program::hasComputeStage() const
{
	return compute != nullptr;
}

Program* Program::createVertex(Shader* vertex, Shader* fragment, const ShaderBindingState* bindings, uint32_t count)
{
	return Application::app()->graphic()->createProgram(vertex, fragment, nullptr, bindings, count);
}
Program* Program::createGeometry(Shader* vertex, Shader* fragment, Shader* geometry, const ShaderBindingState* bindings, uint32_t count)
{
	return Application::app()->graphic()->createProgram(vertex, fragment, geometry, bindings, count);
}
Program* Program::createCompute(Shader* compute)
{
	return nullptr;
}
void Program::destroy(Program* program)
{
	Application::app()->graphic()->destroy(program);
}

};
};