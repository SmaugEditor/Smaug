#include "baseview.h"

void CBaseView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	m_width = width;
	m_height = height;

	m_clearColor = clearColor;

	m_viewId = viewId;
	if (m_viewId != 0)
		m_framebuffer = bgfx::createFrameBuffer(width, height, bgfx::TextureFormat::BGRA8, true);
}

void CBaseView::Update(float dt)
{
	if (m_viewId != 0)
		bgfx::setViewFrameBuffer(m_viewId, m_framebuffer);
	bgfx::setViewRect(m_viewId, 0, 0, (uint16_t)m_width, (uint16_t)m_height);
	bgfx::setViewClear(m_viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, m_clearColor);
	bgfx::setViewMode(m_viewId, bgfx::ViewMode::Sequential);
	bgfx::touch(m_viewId);
}


ImTextureID CBaseView::GetImTextureID()
{
	return (ImTextureID)bgfx::getTexture(m_framebuffer).idx;
}
