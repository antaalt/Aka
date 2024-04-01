#include <Aka/Renderer/Instance/StaticMeshInstance.hpp>

namespace aka {

gfx::VertexBufferLayout StaticMeshInstance::getState()
{
	gfx::VertexBufferLayout attributes{};
	attributes.stepRate = gfx::VertexStepRate::Instance;
	attributes.add(gfx::VertexSemantic::WorldMatrix, gfx::VertexFormat::Float, gfx::VertexType::Vec4);
	attributes.add(gfx::VertexSemantic::WorldMatrix, gfx::VertexFormat::Float, gfx::VertexType::Vec4);
	attributes.add(gfx::VertexSemantic::WorldMatrix, gfx::VertexFormat::Float, gfx::VertexType::Vec4);
	attributes.add(gfx::VertexSemantic::WorldMatrix, gfx::VertexFormat::Float, gfx::VertexType::Vec4);
	attributes.add(gfx::VertexSemantic::NormalMatrix, gfx::VertexFormat::Float, gfx::VertexType::Vec4);
	attributes.add(gfx::VertexSemantic::NormalMatrix, gfx::VertexFormat::Float, gfx::VertexType::Vec4);
	attributes.add(gfx::VertexSemantic::NormalMatrix, gfx::VertexFormat::Float, gfx::VertexType::Vec4);
	attributes.add(gfx::VertexSemantic::NormalMatrix, gfx::VertexFormat::Float, gfx::VertexType::Vec4); // Dummy line should have mat3f aligned on 16 bytes instead
	attributes.add(gfx::VertexSemantic::TexCoord0, gfx::VertexFormat::UnsignedInt, gfx::VertexType::Scalar);
	return attributes;
}

}