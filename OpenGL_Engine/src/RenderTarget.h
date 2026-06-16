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
		
		std::vector<uint32_t> m_ColorAttachments{0,0,0,0,0,0};
		
		uint32_t m_FBO = 0;
		uint32_t m_DepthAttachment = 0;
		uint32_t m_DepthCubeAttachment = 0;
		uint32_t m_DepthStencilAttachment = 0;
		uint32_t m_RenderBuffer = 0;
		
		bool m_bEnableHDR = false;
	public:
		RenderTarget(int width, int height, bool bEnableHDR = false);
		~RenderTarget();

		void CreateColorAttachment(uint32_t slot = 0, uint32_t format = GL_RGB);
		void CreateDepthAttachment();
		void CreateDepthCubeAttachment();
		void CreateDepthStencilAttachment();
		void CreateRenderBuffer();
		
		void BindFramebuffer() const {GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));}
		void UnbindFramebuffer() const {GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));}
		
		void UseMultiColorAttachments(const uint32_t* attachments, const size_t size) const;
		
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
		
		uint32_t GetFBO() const { return m_FBO; }
		uint32_t GetTextureBuffer(int slot = 0) const {return m_ColorAttachments[slot];}
		uint32_t GetDepthBuffer() const {return m_DepthAttachment;}
		uint32_t GetCubeDepthBuffer() const {return m_DepthCubeAttachment;}
		
		void GetRTSize(int& width, int& height) const {width = m_Width; height = m_Height;}
		
		bool CheckEnableHDR() const {return m_bEnableHDR;}
		
		bool CheckFramebufferStatus() const { return m_FBO != 0 && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;  }
	};
}
