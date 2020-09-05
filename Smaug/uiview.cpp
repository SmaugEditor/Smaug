#include "uiview.h"
#include "imgui_internal.h"
#include "objexporter.h"
#include "filesystem.h"

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

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("Export"))
			{
				if (ImGui::MenuItem("Export to OBJ"))
				{
					COBJExporter o;
					char* str = o.Export(&GetWorldEditor());
					filesystem::SaveFileWithDialog(str, "*.obj");
					delete[] str;
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenu();

		}

		ImGui::EndMainMenuBar();
	}

	ImGui::ShowDemoWindow();

	bool hoveredOn2DEditor = false;
	ImVec2 mv, inv, ixv;
	if(ImGui::Begin("2D Editor"))
	{
		m_drawEditView = ImGui::IsWindowCollapsed();
	
		ImVec2 imageSize = ImGui::GetContentRegionAvail();
		m_editView.m_aspectRatio = imageSize.y / imageSize.x;
		ImGui::Image(m_editView.GetImTextureID(), ImGui::GetContentRegionAvail());

		hoveredOn2DEditor = ImGui::IsItemHovered();
		mv = ImGui::GetMousePos();
		inv = ImGui::GetItemRectMin();
		ixv = ImGui::GetItemRectMax();
		ImGui::End();
	}

	ImGui::Begin("3D Preview");
	m_drawPreviewView = ImGui::IsWindowCollapsed();
	ImGui::Image(m_previewView.GetImTextureID(), ImVec2(400, 400));
	bool hoveredOn3DEditor = ImGui::IsItemFocused();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::Begin("Object Editor");
	m_drawSelectedView = ImGui::IsWindowCollapsed();
	ImGui::Image(m_selectedView.GetImTextureID(), ImVec2(400, 400));
	bool hoveredOnSelectionEditor = ImGui::IsItemFocused();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	if (!m_drawEditView && hoveredOn2DEditor)
	{
		float x = (mv.x - inv.x) / (ixv.x - inv.x);
		float y = (mv.y - inv.y) / (ixv.y - inv.y);

		m_editView.Update(dt, x, y);
	}
}
