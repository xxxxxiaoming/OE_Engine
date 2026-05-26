#pragma once

#include <glad/glad.h>
#include <cstdint>
#include "Helper.h"

namespace Engine
{
	class RenderTarget
	{
	private:
		int m_Width;
		int m_Height;
		uint32_t m_FBO = 0;
		uint32_t m_ColorAttachment = 0;
		uint32_t m_DepthAttachment = 0;
		uint32_t m_DepthCubeAttachment = 0;
		uint32_t m_DepthStencilAttachment = 0;
		uint32_t m_RenderBuffer = 0;
	public:
		RenderTarget(int width, int height);
		~RenderTarget();

		void CreateColorAttachment();
		void CreateDepthAttachment();
		void CreateDepthCubeAttachment();
		void CreateDepthStencilAttachment();
		void CreateRenderBuffer();
		
		void BindFramebuffer() const {GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));}
		void UnbindFramebuffer() const {GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));}
		
		void SaveColorAttachment(const std::string& path) const;
		void SaveDepthAttachment(const std::string& path) const;
		void SaveDepthCubeAttachment(const std::string& path) const;
		void DeleteFrameBuffer();
	
		void SetDepthUseOnly()
		{
			GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
			GLCALL(glDrawBuffer(GL_NONE));
			GLCALL(glReadBuffer(GL_NONE));
			GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		}
		uint32_t GetTextureBuffer() const {return m_ColorAttachment;}
		uint32_t GetDepthBuffer() const {return m_DepthAttachment;}
		uint32_t GetCubeDepthBuffer() const {return m_DepthCubeAttachment;}
		
		bool CheckFramebufferStatus() const { return m_FBO != 0 && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;  }
	};
}
