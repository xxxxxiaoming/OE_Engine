#include <glad/glad.h>
#include <stb_image_write.h>
#include <string>
#include <filesystem>
#include "RenderTarget.h"

const std::string pngName[6] = {
	"right.png",
	"left.png",
	"top.png",
	"bottom.png",
	"back.png",
	"front.png"
};

Engine::RenderTarget::RenderTarget(int width, int height, bool bEnableHDR) :
	m_Width(width), m_Height(height) , m_bEnableHDR(bEnableHDR)
{
	GLCALL(glGenFramebuffers(1, &m_FBO));
}

Engine::RenderTarget::~RenderTarget()
{
	DeleteFrameBuffer();
}

void Engine::RenderTarget::CreateDepthAttachment()
{
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
	
	GLCALL(glGenTextures(1, &m_DepthAttachment));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_DepthAttachment));
	
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
	
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	GLCALL(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor));
	
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));
	GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0));
	
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void Engine::RenderTarget::CreateDepthCubeAttachment()
{
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
	GLCALL(glGenTextures(1, &m_DepthCubeAttachment));
	GLCALL(glBindTexture(GL_TEXTURE_CUBE_MAP, m_DepthCubeAttachment));
	
	GLCALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GLCALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
	
	for (unsigned int i = 0; i < 6; ++i)
	{
		GLCALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));
	}
		
	float defaultColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLCALL(glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, defaultColor));
	
	GLCALL(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubeAttachment, 0));
	
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GLCALL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}

void Engine::RenderTarget::CreateColorAttachment(uint32_t slot, uint32_t format)
{
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
	
	GLCALL(glGenTextures(1, &m_ColorAttachments[slot]));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_ColorAttachments[slot]));
	
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	
	uint16_t innerFormat = m_bEnableHDR ? (format == GL_RGB ? GL_RGB16F : GL_RGBA16F) : (format == GL_RGB ? GL_RGB : GL_RGBA);
	uint16_t dataType = m_bEnableHDR ? GL_FLOAT : GL_UNSIGNED_BYTE;
	
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, innerFormat, m_Width, m_Height, 0, format, dataType, nullptr));
	GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_TEXTURE_2D, m_ColorAttachments[slot], 0));
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
	
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void Engine::RenderTarget::CreateDepthStencilAttachment()
{
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
	GLCALL(glGenTextures(1, &m_DepthStencilAttachment));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_DepthStencilAttachment));
	
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
	
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 边缘设为 1.0，代表纯白（在光照下，没有阴影）
	GLCALL(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor));
	
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

void Engine::RenderTarget::UseMultiColorAttachments(const uint32_t* attachments, const size_t size) const
{
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
	
	std::vector<uint32_t>colorAttachments;
	colorAttachments.reserve(size);
	for (size_t i = 0; i < size; ++i)
		colorAttachments.emplace_back(GL_COLOR_ATTACHMENT0 + attachments[i]);
	
	GLCALL(glDrawBuffers(size, colorAttachments.data()));
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void Engine::RenderTarget::SaveColorAttachment(const std::string& path) const
{
	char* pixelBuffer = static_cast<char*>(std::malloc(static_cast<size_t>(m_Width) * static_cast<size_t>(m_Height) * 4));
	
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
	GLCALL(glReadPixels(0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer));
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	
	stbi_flip_vertically_on_write(true);
	stbi_write_png(path.c_str(), m_Width, m_Height, 4, pixelBuffer, m_Width * 4);
	
	std::free(pixelBuffer);
}

void Engine::RenderTarget::SaveDepthAttachment(const std::string& path) const
{
	// 1. 分配正确大小的内存装 float (每个像素 4 个字节)
	float* depthBuffer = static_cast<float*>(std::malloc(static_cast<size_t>(m_Width * m_Height) * sizeof(float)));
	
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
	
	// 2. 格式必须是 GL_DEPTH_COMPONENT，不是 GL_DEPTH_ATTACHMENT
	GLCALL(glReadPixels(0,0,m_Width, m_Height, GL_DEPTH_COMPONENT, GL_FLOAT, depthBuffer));
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	
	// 3. 分配一块专门的内存用来装 8-bit 的单通道图像数据 ( unsigned char )
	unsigned char* imageBuffer = static_cast<unsigned char*>(std::malloc(static_cast<size_t>(m_Width * m_Height)));
	for (int i = 0; i < m_Width * m_Height; ++i)
	{
		// depthBuffer[i] = (2.0f * 0.1f * 3000.0f) / (3000.0f + 0.1f - depthBuffer[i] * (3000.0f - 0.1f));  
		imageBuffer[i] = static_cast<unsigned char>(depthBuffer[i] * 255.0f);
	}
	
	stbi_flip_vertically_on_write(true);
	
	// 4. 将转换好的 imageBuffer 交给 stb 保存
	stbi_write_png(path.c_str(), m_Width, m_Height, 1, imageBuffer, m_Width);
	
	std::free(depthBuffer);
	std::free(imageBuffer);
}

void Engine::RenderTarget::SaveDepthCubeAttachment(const std::string& path) const
{
	float* depthBuffer = static_cast<float*>(std::malloc(static_cast<size_t>(m_Width * m_Height) * sizeof(float)));
	unsigned char* imageBuffer = static_cast<unsigned char*>(std::malloc(static_cast<size_t>(m_Width * m_Height)));
	
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
	
	for (int i = 0; i < 6; ++i)
	{
		GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_DepthCubeAttachment, 0));
		GLCALL(glReadPixels(0, 0, m_Width, m_Height, GL_DEPTH_COMPONENT, GL_FLOAT, depthBuffer));
		
		for (int j = 0; j < m_Width * m_Height; ++j)
		{
			imageBuffer[j] = static_cast<unsigned char>(depthBuffer[j] * 255.0f);
		}
		
		stbi_flip_vertically_on_write(true);
		
		std::string fullPath = path + pngName[i];
		
		// Ensure path existing. If not, create directory.
		std::filesystem::path pathObject{path};
		if (!pathObject.empty() && !std::filesystem::exists(pathObject))
		{
			std::filesystem::create_directory(pathObject);
		}
		
		stbi_write_png(fullPath.c_str(), m_Width, m_Height, 1, imageBuffer, m_Width );
	}
	
	// GLCALL(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubeAttachment, 0));
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	std::free(imageBuffer);
	std::free(depthBuffer);
}

void Engine::RenderTarget::DeleteFrameBuffer()
{
	if (!m_ColorAttachments.empty())
	{
		for (size_t i = 0; i < m_ColorAttachments.size(); ++i)
		{
			if (m_ColorAttachments[i] != 0)
			{
				GLCALL(glDeleteTextures(1, &m_ColorAttachments[i]));
				m_ColorAttachments[i] = 0;
			}
		}
	}
	
	if (m_DepthStencilAttachment != 0)
		GLCALL(glDeleteTextures(1, &m_DepthStencilAttachment));
	
	if (m_RenderBuffer != 0)
		GLCALL(glDeleteRenderbuffers(1, &m_RenderBuffer));
	
	if (m_FBO != 0)
		GLCALL(glDeleteFramebuffers(1, &m_FBO));
	
	m_DepthStencilAttachment = 0;
	m_RenderBuffer = 0;
	m_FBO = 0;
}
