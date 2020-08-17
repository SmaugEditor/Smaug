#include "uiview.h"
#include "imgui_internal.h"

void CUIView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);

	//ImGui::CreateNewWindow

	m_editView.Init(ViewID::EDIT_VIEW, 1024, 1024, 0x00FFFFFF);
	m_previewView.Init(ViewID::PREVIEW_VIEW, 1024, 1024, 0x00FFFFFF);
	m_selectedView.Init(ViewID::SELECTED_VIEW, 1024, 1024, 0x00FFFFFF);

	m_drawPreviewView = true;
	m_drawEditView = true;
	m_drawSelectedView = true;
}

void CUIView::Draw(float dt)
{
	CBaseView::Draw(dt);



	if (!m_drawEditView)
		m_editView.Draw(dt);
	if(!m_drawPreviewView)
		m_previewView.Draw(dt);
	if (!m_drawSelectedView)
		m_selectedView.Draw(dt);


}

void CUIView::Update(float dt, float mx, float my)
{
	ImGui::ShowDemoWindow();

	ImGui::Begin("2D Editor");
	m_drawEditView = ImGui::IsWindowCollapsed();
	ImGui::Image(m_editView.GetImTextureID(), ImVec2(400, 400));
	ImVec2 mv = ImGui::GetMousePos();
	ImVec2 inv = ImGui::GetItemRectMin();
	ImVec2 ixv = ImGui::GetItemRectMax();
	ImGui::End();

	ImGui::Begin("3D Preview");
	m_drawPreviewView = ImGui::IsWindowCollapsed();
	ImGui::Image(m_previewView.GetImTextureID(), ImVec2(400, 400));
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::Begin("Object Editor");
	m_drawSelectedView = ImGui::IsWindowCollapsed();
	ImGui::Image(m_selectedView.GetImTextureID(), ImVec2(400, 400));
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	if (!m_drawEditView)
	{
		ImGui::GetIO().WantCaptureKeyboard = false;
		bool wantCapture = ImGui::GetIO().WantCaptureMouse;
		float x = (mv.x - inv.x) / (ixv.x - inv.x);
		float y = (mv.y - inv.y) / (ixv.y - inv.y);
		// Probably a better way to do this
		if (x >= 0 && x <= 1 && y >= 0 && y <= 1)
			ImGui::GetIO().WantCaptureMouse = false;
		m_editView.Update(dt, x, y);
		ImGui::GetIO().WantCaptureMouse = wantCapture;
	}

}
