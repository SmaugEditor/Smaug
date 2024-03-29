#include "selectionmanager.h"
#include "cursor.h"
#include "settingsmenu.h"
#include "svarex.h"
#include "utils.h"
#include "doodle.h"
#include <imgui.h>

BEGIN_SVAR_TABLE(CSelectionSettings)
DEFINE_TABLE_SVAR_INPUT(select, MOUSE_1, true)
DEFINE_TABLE_SVAR_INPUT(multiselectmode, KEY_LEFT_CONTROL, false)
END_SVAR_TABLE()

static CSelectionSettings s_selectionSettings;
DEFINE_SETTINGS_MENU("Selection", s_selectionSettings);

void CSelectionManager::Update2D(glm::vec3 mousePos)
{
	if (Input().IsDown(s_selectionSettings.select))
	{
		SelectionManager().SelectAtPoint(mousePos, SelectionMode());
	}
}

void CSelectionManager::Update3D(testWorld_t worldTest)
{
	if (worldTest.hit)
	{
		if (Input().IsDown(s_selectionSettings.select))
		{
			selectionInfo_t si = {
				.selected = SelectionFlag::SF_PART,
				.node = CNodeRef(worldTest.node),
				.part = CNodeMeshPartRef(worldTest.part, worldTest.node)
			};
			AddSelection(si);
		}
	}
}

void CSelectionManager::Draw()
{
	for (auto s : m_selected)
	{
		if (s.part.IsValid())
			Doodle().Face(s.part, COLOR_GREEN);
		if (s.edge.IsValid())
		{
			glm::vec3 start = *s.edge->vert + s.node.Node()->Origin();
			glm::vec3 end = *s.edge->edge->vert->vert + s.node.Node()->Origin();
			Doodle().Line(start, end, COLOR_GREEN);
		}
	}

}

bool CSelectionManager::MultiselectMode()
{
	return Input().IsDown(s_selectionSettings.multiselectmode);
}

bool CSelectionManager::SelectAtPoint(glm::vec3 point, SelectionFlag search)
{
	// We successfully found nothing! Woo!
	if (search == SelectionFlag::SF_NONE)
		return true;

	selectionInfo_t outInfo;


	// What plane are we editing on?
	glm::vec3 workingAxis = GetCursor().GetWorkingAxis();
	glm::vec3 workingAxisMask = GetCursor().GetWorkingAxisMask();


	float vertDist = FLT_MAX;
	// We can do this because part normals are normalized
	// So our T ends up acting almost like a distance
	float partT = FLT_MAX;


	for (auto pair : GetWorldEditor().m_nodes)
	{
		selectionInfo_t info;

		CNode* node = pair.second;

		// Put the mouse on level with the node
		glm::vec3 localMouse = point - node->Origin();
		localMouse *= workingAxisMask; // Flatten it out to just this plane
		aabb_t aabb = node->GetLocalAABB();

		// Check if we're in the AABB
		if (!testPointInAABB(localMouse, aabb, 0.5f))
			continue; // Not in bounds!

		bool didSet = false;
		for (auto part : node->m_mesh.parts)
		{

			if (search & SelectionFlag::SF_EDGE)
			{
				for (auto v : part->verts)
				{
					// Not a fantastic way to test this...
					// Are we on both parts of the edge?
					float vdist = glm::distance(localMouse, *v->vert * workingAxisMask) +
						glm::distance(localMouse, *v->edge->vert->vert * workingAxisMask);
					if (vdist <= 1.0f && vdist < vertDist)
					{
						vertDist = vdist;
						info.edge = { v, node };
						didSet = true;
						info.selected |= SelectionFlag::SF_EDGE;
					}
				}
			}

			if (search & SelectionFlag::SF_PART)
			{
				// Discard parts with norms we don't like
				if (fabs(glm::dot(part->normal, workingAxis)) > 0.89)
				{
					continue;
				}
				
				testRayPlane_t t = pointOnPartLocal(part, localMouse);

				// We want to allow for a lot more grab room within the node. Prevents mistakes from being made.
				float abT = fabs(t.t);

				const float threshold = 1.0f;
				if (t.hit && abT < partT && abT <= threshold)
				{
					partT = abT;
					info.part = { part, node };
					didSet = true;
					info.selected |= SelectionFlag::SF_PART;
				}
			}

		}

		if (search == SelectionFlag::SF_NODE)
		{
			info.node = node;
			info.selected |= SelectionFlag::SF_NODE;
		}
		else 
		{
			if (didSet)
				info.node = node;
		}

		if (didSet)
			outInfo = info;
	}

	if (outInfo.selected != SelectionFlag::SF_NONE)
	{
		AddSelection(outInfo);
		return true;
	}
	return false;
}

void CSelectionManager::AddSelection(selectionInfo_t& si)
{
	if (MultiselectMode())
	{
		bool matched = false;
		for (auto i : m_selected)
			if (si.selected == i.selected
				&& si.node.ID() == i.node.ID()
				&& si.edge.ID() == i.edge.ID()
				&& si.part.ID() == i.part.ID())
			{
				matched = true;
				break;
			}
		if (!matched)
			m_selected.push_back(si);
	}
	else
	{
		m_selected.clear();
		m_selected.push_back(si);
	}
}


void CSelectionManager::ShowSelectionMenu()
{
	// Simplify this?
	if (ImGui::Begin("Selection Mode"))
	{
		bool push = false;
		const ImVec4 selectColor = { 128, 0, 0, 255 };
		if (m_selectionMode & SelectionFlag::SF_NODE)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, selectColor);
			push = true;
		}
		if (ImGui::Button("Node"))
			m_selectionMode = SelectionFlag::SF_NODE;
		if (push)
		{
			ImGui::PopStyleColor(1);
			push = false;
		}


		if (m_selectionMode & SelectionFlag::SF_PART)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, selectColor);
			push = true;
		}
		if (ImGui::Button("Part"))
			m_selectionMode = SelectionFlag::SF_PART;
		if (push)
		{
			ImGui::PopStyleColor(1);
			push = false;
		}
		
		

		if (m_selectionMode & SelectionFlag::SF_EDGE)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, selectColor);
			push = true;
		}
		if (ImGui::Button("Edge"))
			m_selectionMode = SelectionFlag::SF_EDGE;
		if (push)
		{
			ImGui::PopStyleColor(1);
			push = false;
		}
	}
	ImGui::End();
}


CSelectionManager& SelectionManager()
{
	static CSelectionManager s_selectionManager;
	return s_selectionManager;
}
