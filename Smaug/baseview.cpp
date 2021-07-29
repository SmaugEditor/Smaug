#include "baseview.h"

void CBaseView::Init(uint16_t viewId, int width, int height, uint32_t clearColor)
{
	m_width = width;
	m_height = height;

	m_clearColor = clearColor;

	if(viewId > 0)
		m_renderTarget = EngineInterface()->CreateRenderTarget(width, height);
	
	m_focused = false;
}

void CBaseView::Draw(float dt)
{
	
}


ImTextureID CBaseView::GetImTextureID()
{
	return reinterpret_cast<ImTextureID>(EngineInterface()->TextureFromRenderTarget(m_renderTarget));
}
