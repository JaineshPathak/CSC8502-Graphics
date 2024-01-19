#include "ShadowBuffer.h"
#include <iostream>
#include <glad/glad.h>

ShadowBuffer::ShadowBuffer(const int& width, const int& height) : 
	m_Width(width), m_Height(height)
{
	//Creating Depth Texture
	glGenTextures(1, &m_DepthTexture);
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Creating Frame Buffer
	glGenFramebuffers(1, &m_DepthFrameBufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_DepthFrameBufferID);
	//Attaching the Depth Texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Shadow Buffer: Ready" << std::endl;

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowBuffer::~ShadowBuffer()
{
	glDeleteTextures(1, &m_DepthTexture);
	glDeleteFramebuffers(1, &m_DepthFrameBufferID);
}

void ShadowBuffer::Bind()
{
	glViewport(0, 0, m_Width, m_Height);
	glBindFramebuffer(GL_FRAMEBUFFER, m_DepthFrameBufferID);
}

void ShadowBuffer::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
