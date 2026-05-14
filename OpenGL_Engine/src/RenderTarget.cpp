#include <glad/glad.h>
#include <stb_image_write.h>
#include <string>
#include "RenderTarget.h"

#include <chrono>

Engine::RenderTarget::RenderTarget(int width, int height) :
	m_Width(width), m_Height(height)
{
	GLCALL(glGenFramebuffers(1, &m_FBO));
}

Engine::RenderTarget::~RenderTarget()
{
	DeleteFrameBuffer();
}

void Engine::RenderTarget::CreateColorAttachment()
{
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
	
	GLCALL(glGenTextures(1, &m_ColorAttachment));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_ColorAttachment));
	
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));
	GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0));
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
	
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void Engine::RenderTarget::CreateDepthStencilAttachment()
{
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
	GLCALL(glGenTextures(1, &m_DepthStencilAttachment));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_DepthStencilAttachment));
	
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthStencilAttachment, 0));
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void Engine::RenderTarget::CreateRenderBuffer()
{
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
	GLCALL(glGenRenderbuffers(1, &m_RenderBuffer));
	GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffer));
	GLCALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, m_Height ));
	GLCALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBuffer ));
	GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void Engine::RenderTarget::SaveColorAttachment() const
{
	char* pixelBuffer = static_cast<char*>(std::malloc(static_cast<size_t>(m_Width) * static_cast<size_t>(m_Height) * 4));
	
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
	GLCALL(glReadPixels(0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer));
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	
	stbi_flip_vertically_on_write(true);
	
	std::string path = "res/screenshot/screenshot.png";
	stbi_write_png(path.c_str(), m_Width, m_Height, 4, pixelBuffer, m_Width * 4);
}

void Engine::RenderTarget::DeleteFrameBuffer()
{
	if (m_ColorAttachment != 0)
		GLCALL(glDeleteTextures(1, &m_ColorAttachment));
	
	if (m_DepthStencilAttachment != 0)
		GLCALL(glDeleteTextures(1, &m_DepthStencilAttachment));
	
	if (m_RenderBuffer != 0)
		GLCALL(glDeleteRenderbuffers(1, &m_RenderBuffer));
	
	if (m_FBO != 0)
		GLCALL(glDeleteFramebuffers(1, &m_FBO));
	
	m_ColorAttachment = 0;
	m_DepthStencilAttachment = 0;
	m_RenderBuffer = 0;
	m_FBO = 0;
}
