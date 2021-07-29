#include "selectedview.h"

#include "worldeditor.h"
#include "actionmanager.h"
#include "grid.h"

#include "editorinterface.h"

#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include "imgui_internal.h"

void CSelectedView::Init(uint16_t viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);
	m_selectedNode = nullptr;
}


void CSelectedView::Draw(float dt)
{
	EngineInterface()->BeginView(m_renderTarget);
	EngineInterface()->ClearColor(m_renderTarget, m_clearColor);

	CBaseView::Draw(dt);
	float time = EngineInterface()->Time();

	if (m_selectedNode.IsValid())
	{
		
		float distance = m_aabbLength;
		glm::vec3 origin = m_selectedNode->m_mesh.origin;
		glm::mat4 view = glm::lookAt(glm::vec3(cos(time) * -distance, distance, sin(time) * distance) + origin, origin, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 proj = glm::perspective(glm::radians(60.0f), m_aspectRatio, 0.1f, 800.0f);
		EngineInterface()->SetViewMatrix(view, proj);

		m_selectedNode->m_renderData->Draw();
	}

	EngineInterface()->EndView(m_renderTarget);
}

void CSelectedView::SetSelection(CNodeRef node)
{
	aabb_t aabb = node->GetLocalAABB();
	m_aabbLength = glm::distance(aabb.min, aabb.max);
	m_selectedNode = node;
}

bool CSelectedView::Show()
{
	bool drawNextFrame = true;


	ImGuiID id = ImGui::GetID("SelectedViewDS");
	// No if. If we have one, everything pops out of it's slot
	ImGui::Begin("Object Editor");
	{
		drawNextFrame = !ImGui::IsWindowCollapsed();
		ImGui::DockSpace(id);
		
		static bool once = true;
		if (once)
		{
			once = false;
			ImGui::DockBuilderRemoveNode(id); // clear any previous layout
			ImGui::DockBuilderAddNode(id, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_NoTabBar);
			ImGui::DockBuilderSetNodeSize(id, ImGui::GetWindowSize());

			// Split the node so our window has two sections and is resizable
			ImGuiID up, down;
			ImGui::DockBuilderSplitNode(id, ImGuiDir_Up, 0.45, &up, &down);
			ImGui::DockBuilderGetNode(up)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
			ImGui::DockBuilderGetNode(down)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

			ImGui::DockBuilderDockWindow("SelectedViewPreview", up);
			ImGui::DockBuilderDockWindow("SelectedViewProperties", down);
			ImGui::DockBuilderFinish(id);
		}
	}
	ImGui::End();

	if (ImGui::Begin("SelectedViewPreview", 0, ImGuiWindowFlags_NoTitleBar))
	{
		ImVec2 imageSize = ImGui::GetContentRegionAvail();
		m_aspectRatio = imageSize.x / imageSize.y;
		ImGui::Image(GetImTextureID(), imageSize);
	}
	ImGui::End();


	if (ImGui::Begin("SelectedViewProperties", 0, ImGuiWindowFlags_NoTitleBar))
	{
		if (m_selectedNode.IsValid())
		{
			bool vis = m_selectedNode->IsVisible();
			if(ImGui::Checkbox("Visible", &vis))
			{
				m_selectedNode->SetVisible(vis);
			}
			ImGui::DragFloat3("Origin", reinterpret_cast<float*>(&m_selectedNode->m_mesh.origin));

		}
	}
	ImGui::End();
	return drawNextFrame;
}

CSelectedView& SelectedView()
{
	static CSelectedView s_sv;
	return s_sv;
}
