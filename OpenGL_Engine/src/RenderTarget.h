#pragma once

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
		uint32_t m_DepthStencilAttachment = 0;
		uint32_t m_RenderBuffer = 0;
	public:
		RenderTarget(int width, int height);
		~RenderTarget();

		void CreateColorAttachment();
		void CreateDepthStencilAttachment();
		void CreateRenderBuffer();

		inline void BindFramebuffer() const {GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));}
		inline void UnbindFramebuffer() const {GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));}
		
		void SaveColorAttachment() const;
		void DeleteFrameBuffer();
	
		inline const uint32_t GetTextureBuffer() const {return m_ColorAttachment;}
		
		inline bool CheckFramebufferStatus() const { return m_FBO != 0 && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;  }
	};
}
