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
		uint32_t m_DepthStencilAttachment = 0;
		uint32_t m_RenderBuffer = 0;
	public:
		RenderTarget(int width, int height);
		~RenderTarget();

		void CreateColorAttachment();
		void CreateDepthAttachment();
		void CreateDepthStencilAttachment();
		void CreateRenderBuffer();

		inline void BindFramebuffer() const {GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));}
		inline void UnbindFramebuffer() const {GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));}
		
		void SaveColorAttachment(const std::string& path) const;
		void SaveDepthAttachment(const std::string& path) const;
		void DeleteFrameBuffer();
	
		inline void SetDepthUseOnly()
		{
			GLCALL(glDrawBuffer(GL_NONE));
			GLCALL(glReadBuffer(GL_NONE));
		}
		inline uint32_t GetTextureBuffer() const {return m_ColorAttachment;}
		inline uint32_t GetDepthBuffer() const {return m_DepthAttachment;}
		
		inline bool CheckFramebufferStatus() const { return m_FBO != 0 && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;  }
	};
}
