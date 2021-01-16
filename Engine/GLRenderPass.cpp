#include "RenderPass.h"

#include "GLBackend.h"

namespace aka {

void RenderPass::execute()
{
	{
		// Set framebuffer
		if (framebuffer != nullptr)
		{
			framebuffer->bind(Framebuffer::Type::Both);
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	{
		// Blending
		if (blend == BlendMode::None)
		{
			glDisable(GL_BLEND);
		}
		else
		{
			glEnable(GL_BLEND);
			switch (blend)
			{
			case BlendMode::Zero:
				glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
				break;
			case BlendMode::One:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				break;
			case BlendMode::SrcColor:
				glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
				break;
			case BlendMode::OneMinusSrcColor:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
				break;
			case BlendMode::DstColor:
				glBlendFunc(GL_SRC_ALPHA, GL_DST_COLOR);
				break;
			case BlendMode::OneMinusDstColor:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_COLOR);
				break;
			case BlendMode::SrcAlpha:
				glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
				break;
			case BlendMode::OneMinusSrcAlpha:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BlendMode::DstAlpha:
				glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
				break;
			case BlendMode::OneMinusDstAlpha:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
				break;
			case BlendMode::ConstantColor:
				glBlendFunc(GL_SRC_ALPHA, GL_CONSTANT_COLOR);
				break;
			case BlendMode::OneMinusConstantColor:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_CONSTANT_COLOR);
				break;
			case BlendMode::ConstantAlpha:
				glBlendFunc(GL_SRC_ALPHA, GL_CONSTANT_ALPHA);
				break;
			case BlendMode::OneMinusConstantAlpha:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
				break;
			case BlendMode::SrcAlphaSaturate:
				glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA_SATURATE);
				break;
			case BlendMode::Src1Color:
				glBlendFunc(GL_SRC_ALPHA, GL_SRC1_COLOR);
				break;
			case BlendMode::OneMinusSrc1Color:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC1_COLOR);
				break;
			case BlendMode::Src1Alpha:
				glBlendFunc(GL_SRC_ALPHA, GL_SRC1_ALPHA);
				break;
			case BlendMode::OneMinusSrc1Alpha:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC1_ALPHA);
				break;
			}
		}
	}
	{
		// Cull
		if (cull == CullMode::None)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
			switch (cull)
			{
			case CullMode::FrontFace:
				glCullFace(GL_FRONT);
				glFrontFace(GL_CCW);
				break;
			case CullMode::BackFace:
				glCullFace(GL_BACK);
				glFrontFace(GL_CCW);
				break;
			case CullMode::AllFace:
				glCullFace(GL_FRONT_AND_BACK);
				glFrontFace(GL_CCW);
				break;
			}
		}
	}

	{
		// TODO set depth
	}

	{
		// Viewport
		glViewport(static_cast<GLint>(viewport.x), static_cast<GLint>(viewport.y), static_cast<GLint>(viewport.w), static_cast<GLint>(viewport.h));
	}

	{
		// TODO Scissor
	}

	{
		// Shader
		if (shader == nullptr)
		{
			Logger::error("No shader set for render pass");
			return;
		}
		else
		{
			shader->use();
			if (texture != nullptr)
			{
				glActiveTexture(GL_TEXTURE0);
				texture->bind();
			}
			else
			{
				uint8_t data[4] = { 255, 255, 255, 255 };
				texture = Texture::create(1, 1, Texture::Format::Rgba8, Texture::Format::Rgba, data, Sampler::Filter::Nearest);
				glActiveTexture(GL_TEXTURE0);
				texture->bind();
			}
		}
	}
	{
		// Mesh
		if (mesh == nullptr)
		{
			Logger::error("No mesh set for render pass");
			return;
		}
		else
		{
			glBindVertexArray(mesh->id());
			// Get index size & format from mesh
			uint32_t indexSize = mesh->indexSize();
			GLenum indexFormat = gl::format(mesh->indexFormat());
			// We could use glDrawArrays for non indexed array.
			// But using indexed allow to reduce the number of draw call for different types
			// We can also reuse vertices with glDrawElements as they are indexed
			glDrawElements(
				GL_TRIANGLES,
				static_cast<GLsizei>(indexCount),
				indexFormat,
				(void*)(indexSize * indexOffset)
			);
			glBindVertexArray(0);
		}
	}
}

};