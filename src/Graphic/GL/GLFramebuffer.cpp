#if defined(AKA_USE_OPENGL)
#include "GLFramebuffer.h"

#include "GLContext.h"

#include <Aka/OS/Logger.h>

namespace aka {

GLFramebuffer::GLFramebuffer(Attachment* attachments, size_t count) :
	Framebuffer(attachments, count),
	m_framebufferID(0)
{
	glGenFramebuffers(1, &m_framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
	std::vector<GLenum> drawBuffers;
	for (size_t iAtt = 0; iAtt < count; iAtt++)
	{
		attach(attachments[iAtt]);
		GLenum attachmentType = glAttachmentType(attachments[iAtt].type);
		if (attachmentType >= GL_COLOR_ATTACHMENT0 && attachmentType <= GL_COLOR_ATTACHMENT15)
			drawBuffers.push_back(attachmentType);
	}
	AKA_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not created");
	glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
}

GLFramebuffer::~GLFramebuffer()
{
	if (m_framebufferID != 0)
		glDeleteFramebuffers(1, &m_framebufferID);
}

void GLFramebuffer::clear(const color4f& color, float depth, int stencil, ClearMask mask)
{
	if (mask == ClearMask::None)
		return;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebufferID);
	GLenum glMask = 0;
	if ((mask & ClearMask::Color) == ClearMask::Color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
		glMask |= GL_COLOR_BUFFER_BIT;
	}
	if ((mask & ClearMask::Depth) == ClearMask::Depth)
	{
		glDepthMask(true); // Ensure we can correctly clear the depth buffer.
		glClearDepth(depth);
		glMask |= GL_DEPTH_BUFFER_BIT;
	}
	if ((mask & ClearMask::Stencil) == ClearMask::Stencil)
	{
		glClearStencil(stencil);
		glMask |= GL_STENCIL_BUFFER_BIT;
	}
	glClear(glMask);
}

void GLFramebuffer::set(AttachmentType type, Texture::Ptr texture, AttachmentFlag flag, uint32_t layer, uint32_t level)
{
	// Check attachment
	Attachment newAttachment = Attachment{ type, texture, flag, layer, level };
	if (!valid(newAttachment))
	{
		Logger::error("Incompatible attachment set for framebuffer");
		return;
	}
	Attachment* attachment = getAttachment(type);
	if (attachment == nullptr)
	{
		m_attachments.push_back(newAttachment);
		attachment = &m_attachments.back();
		// Set correct draw buffers to support new attachment
		std::vector<GLenum> drawBuffers;
		for (Attachment& attachment : m_attachments)
		{
			GLenum attachmentType = glAttachmentType(attachment.type);
			if (attachmentType >= GL_COLOR_ATTACHMENT0 && attachmentType <= GL_COLOR_ATTACHMENT15)
				drawBuffers.push_back(attachmentType);
		}
		glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
	}
	else
	{
		if (attachment->texture == texture && attachment->flag == flag && attachment->layer == layer && attachment->level == level)
			return; // Everything already set.
		attachment->texture = texture;
		attachment->flag = flag;
		attachment->layer = layer;
		attachment->level = level;
	}
	// Attach new attachment to framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
	attach(*attachment);
	AKA_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not valid");

	// Recompute size
	computeSize();
}
void GLFramebuffer::attach(const Attachment& attachment)
{
	GLenum attachmentType = glAttachmentType(attachment.type);
	GLuint textureID = (GLuint)attachment.texture->handle().value();
	switch (attachment.texture->type())
	{
	case TextureType::Texture1D:
		glFramebufferTexture1D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_1D, textureID, attachment.level);
		break;
	case TextureType::Texture2D:
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, textureID, attachment.level);
		break;
	case TextureType::Texture3D:
		glFramebufferTexture3D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_3D, textureID, attachment.level, attachment.layer);
		break;
	case TextureType::TextureCubeMap:
		if ((AttachmentFlag::AttachTextureObject & attachment.flag) == AttachmentFlag::AttachTextureObject)
			glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, textureID, attachment.level);
		else
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_CUBE_MAP_POSITIVE_X + attachment.layer, textureID, attachment.level);
		break;
	case TextureType::Texture1DArray:
	case TextureType::Texture2DArray:
		if ((AttachmentFlag::AttachTextureObject & attachment.flag) == AttachmentFlag::AttachTextureObject)
			glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, textureID, attachment.level);
		else
			glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentType, textureID, attachment.level, attachment.layer);
		break;
	case TextureType::TextureCubeMapArray:
		if ((AttachmentFlag::AttachTextureObject & attachment.flag) == AttachmentFlag::AttachTextureObject)
			glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentType, textureID, attachment.level, attachment.layer);
		else
		{
			// TODO allow to attach single face ?
		}
		break;
	}
}
void GLFramebuffer::computeSize()
{
	m_width = m_attachments[0].texture->width();
	m_height = m_attachments[0].texture->height();

	for (size_t i = 1; i < m_attachments.size(); ++i)
	{
		if (m_width > m_attachments[i].texture->width())
			m_width = m_attachments[i].texture->width();
		if (m_height > m_attachments[i].texture->height())
			m_height = m_attachments[i].texture->height();
	}
}

};

#endif