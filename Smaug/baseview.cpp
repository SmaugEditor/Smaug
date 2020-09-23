#include "baseview.h"

void CBaseView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	m_width = width;
	m_height = height;

	m_clearColor = clearColor;

	m_viewId = viewId;
	if (m_viewId != 0)
	{

		m_fbColorTexture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, (uint64_t( 1) << BGFX_TEXTURE_RT_MSAA_SHIFT) | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
		
		uint64_t textureFlags = BGFX_TEXTURE_RT_WRITE_ONLY;
		bgfx::TextureFormat::Enum depthFormat =
			bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D16, textureFlags) ? bgfx::TextureFormat::D16
			: bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D24S8, textureFlags) ? bgfx::TextureFormat::D24S8
			: bgfx::TextureFormat::D32;

		m_fbDepthTexture = bgfx::createTexture2D(width, height, false, 1, depthFormat, textureFlags);


		m_framebuffer = bgfx::createFrameBuffer(2, m_fbTextures, true);
	}
}

void CBaseView::Draw(float dt)
{
	if (m_viewId != 0)
		bgfx::setViewFrameBuffer(m_viewId, m_framebuffer);
	bgfx::setViewRect(m_viewId, 0, 0, (uint16_t)m_width, (uint16_t)m_height);
	bgfx::setViewClear(m_viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, m_clearColor);

	bgfx::touch(m_viewId);
}


ImTextureID CBaseView::GetImTextureID()
{
	return (ImTextureID)bgfx::getTexture(m_framebuffer).idx;
}
