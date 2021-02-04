#include "uiview.h"
#include "imgui_internal.h"
#include "objexporter.h"
#include "vmfexporter.h"
#include "filesystem.h"
#include "basetool.h"
#include "cursor.h"
#include "nodetools.h"
#include "texturebrowser.h"

void CUIView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);

	//ImGui::CreateNewWindow

	// Move this?
	GetSettingsRegister().LoadSettings();

	m_editView.Init(ViewID::EDIT_VIEW, 1024, 1024, 0x383838FF);
	m_previewView.Init(ViewID::PREVIEW_VIEW, 1024, 1024, 0x383838FF);
	m_selectedView.Init(ViewID::SELECTED_VIEW, 1024, 1024, 0x383838FF);

	m_drawPreviewView = true;
	m_drawEditView = true;
	m_drawSelectedView = true;

	m_toolBox.RegisterTool(new CDragTool());
	m_toolBox.RegisterTool(new CExtrudeTool());

}

void CUIView::Draw(float dt)
{
	CBaseView::Draw(dt);



	if (m_drawEditView)
		m_editView.Draw(dt);
	if (m_drawPreviewView)
		m_previewView.Draw(dt);
	if (m_drawSelectedView)
		m_selectedView.Draw(dt);


}

void CUIView::Update(float dt, float mx, float my)
{
	GetCursor().Update(dt);
	ImVec2 mv = ImGui::GetMousePos();

	// UI
#ifdef _DEBUG
	ImGui::ShowDemoWindow();
#endif

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("Export"))
			{
				if (ImGui::MenuItem("Export to VMF"))
				{
					CVMFExporter o;
					char* str = o.Export(&GetWorldEditor());
					filesystem::SaveFileWithDialog(str, "*.vmf");
					delete[] str;
				}

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
		if (ImGui::BeginMenu("Options"))
		{
			if (ImGui::MenuItem("Settings"))
			{
				m_settingsMenu.Enable();
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if(ImGui::Begin("2D Editor"))
	{
		m_drawEditView = !ImGui::IsWindowCollapsed();
	
		ImVec2 imageSize = ImGui::GetContentRegionAvail();
		m_editView.m_aspectRatio = imageSize.x / imageSize.y;
		ImGui::Image(m_editView.GetImTextureID(), imageSize);

		bool hoveredOn2DEditor = ImGui::IsItemHovered();
		ImVec2 inv = ImGui::GetItemRectMin();
		ImVec2 ixv = ImGui::GetItemRectMax();
		
		ImGui::End();

		if (m_drawEditView && hoveredOn2DEditor && !m_previewView.m_controllingCamera)
		{
			float x = (mv.x - inv.x) / (ixv.x - inv.x);
			float y = (mv.y - inv.y) / (ixv.y - inv.y);

			m_editView.Update(dt, x, y);
			m_toolBox.Update(dt, m_editView.TransformMousePos(x, y));

		}
	}

	if(ImGui::Begin("3D Preview"))
	{
		m_drawPreviewView = !ImGui::IsWindowCollapsed();
		
		ImVec2 imageSize = ImGui::GetContentRegionAvail();
		m_previewView.m_aspectRatio = imageSize.x / imageSize.y;
		ImGui::Image(m_previewView.GetImTextureID(), imageSize);
		
		bool hoveredOn3DPreview = ImGui::IsItemHovered();
		ImVec2 inv = ImGui::GetItemRectMin();
		ImVec2 ixv = ImGui::GetItemRectMax();
		
		ImGui::End();

		if (m_drawPreviewView && (hoveredOn3DPreview || m_previewView.m_controllingCamera))
		{
			float x = (mv.x - inv.x) / (ixv.x - inv.x);
			float y = (mv.y - inv.y) / (ixv.y - inv.y);

			m_previewView.Update(dt, x, y);
		}
	}

	if (ImGui::Begin("Object Editor"))
	{
		m_drawSelectedView = !ImGui::IsWindowCollapsed();
		ImVec2 imageSize = ImGui::GetContentRegionAvail();
		m_selectedView.m_aspectRatio = imageSize.x / imageSize.y;
		ImGui::Image(m_selectedView.GetImTextureID(), imageSize);
		bool hoveredOnSelectionEditor = ImGui::IsItemHovered();
		ImGui::End();
	}

	if (ImGui::Begin("Edit History"))
	{
		std::vector<IAction*>& actionHistory = GetActionManager().m_actionHistory;

		for (int i = 0; i < actionHistory.size(); i++)
		{
			ImGui::Text(actionHistory[i]->GetName());
		}
		ImGui::End();
	}

	 
	m_toolBox.ShowToolBox();
	m_settingsMenu.DrawMenu();
	TextureBrowser().Show();
}
