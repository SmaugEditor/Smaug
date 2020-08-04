#include "uiview.h"

void CUIView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);


	m_previewView.Init(ViewID::PREVIEW_VIEW, 1024, 1024, 0x00000000);
}

void CUIView::Update(float dt)
{
	m_previewView.Update(dt);

	CBaseView::Update(dt);

	ImGui::ShowDemoWindow();

	ImGui::Begin("2D Editor");
	ImGui::End();

	ImGui::Begin("3D Preview");
	ImGui::Image(m_previewView.GetImTextureID(), ImVec2(400, 400));
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}
