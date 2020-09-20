#include "uiview.h"
#include "imgui_internal.h"
#include "objexporter.h"
#include "vmfexporter.h"
#include "filesystem.h"
#include "basetool.h"
#include "cursor.h"
#include "nodetools.h"

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

	m_toolBox.m_tools.push_back(new CDragTool());
	m_toolBox.m_tools.push_back(new CExtrudeTool());
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

	// UI

	ImGui::ShowDemoWindow();

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

		ImGui::EndMainMenuBar();
	}

	bool hoveredOn2DEditor = false;
	ImVec2 mv, inv, ixv;
	if(ImGui::Begin("2D Editor"))
	{
		m_drawEditView = !ImGui::IsWindowCollapsed();
	
		ImVec2 imageSize = ImGui::GetContentRegionAvail();
		m_editView.m_aspectRatio = imageSize.x / imageSize.y;
		ImGui::Image(m_editView.GetImTextureID(), imageSize);

		hoveredOn2DEditor = ImGui::IsItemHovered();
		mv = ImGui::GetMousePos();
		inv = ImGui::GetItemRectMin();
		ixv = ImGui::GetItemRectMax();
		ImGui::End();
	}

	bool hoveredOn3DPreview = false;
	if(ImGui::Begin("3D Preview"))
	{
		m_drawPreviewView = !ImGui::IsWindowCollapsed();
		
		ImVec2 imageSize = ImGui::GetContentRegionAvail();
		m_previewView.m_aspectRatio = imageSize.x / imageSize.y;
		ImGui::Image(m_previewView.GetImTextureID(), imageSize);
		
		hoveredOn3DPreview = ImGui::IsItemHovered();
		ImGui::End();
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

	if (m_drawEditView && hoveredOn2DEditor && !m_previewView.m_controllingCamera)
	{
		float x = (mv.x - inv.x) / (ixv.x - inv.x);
		float y = (mv.y - inv.y) / (ixv.y - inv.y);
		
		m_editView.Update(dt, x, y);
		m_toolBox.Update(dt, m_editView.TransformMousePos(x, y));

	}

	if (m_drawPreviewView && (hoveredOn3DPreview || m_previewView.m_controllingCamera))
	{
		m_previewView.Update(dt, 0, 0);
	}
}
