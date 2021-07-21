#include "selectionmanager.h"
#include "cursor.h"
#include "raytest.h"
#include "debugdraw.h"
#include "settingsmenu.h"
#include "svarex.h"

#include <GLFW/glfw3.h>
#include <basicdraw.h>

BEGIN_SVAR_TABLE(CSelectionSettings)
DEFINE_TABLE_SVAR_INPUT(select, GLFW_MOUSE_BUTTON_1, true)
DEFINE_TABLE_SVAR_INPUT(multiselectmode, GLFW_KEY_LEFT_CONTROL, false)
END_SVAR_TABLE()

static CSelectionSettings s_selectionSettings;
DEFINE_SETTINGS_MENU("Selection", s_selectionSettings);

void CSelectionManager::Update2D(glm::vec3 mousePos)
{
	if (MultiselectMode())
	{
		// If we found something, and we're clicking, we can start dragging!
		// Using glfw input until something is figured out about ImGui's overriding
		if (Input().IsDown(s_selectionSettings.select))
		{
			// Check if we're hovering on anything
			SelectionManager().SelectAtPoint(mousePos, SelectionMode());
		}
		GetCursor().SetPosition(mousePos);
	}
	else
	{
		if (Input().IsDown(s_selectionSettings.select))
		{
			SelectionManager().SelectAtPoint(mousePos, SelectionMode());
		}
	}
}

void CSelectionManager::Update3D(ray_t mouseRay)
{
	testWorld_t t = testRay(mouseRay);
	if (t.hit)
	{
		GetCursor().SetPositionForce(t.intersect);
		if (Input().IsDown(s_selectionSettings.select))
		{
			selectionInfo_t si = {
				.selected = SelectionFlag::SF_PART,
				.node = CNodeRef(t.node),
				.part = CNodeMeshPartRef(t.part, t.node)
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
			BasicDraw().Face(s.part, COLOR_GREEN);

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

CSelectionManager& SelectionManager()
{
	static CSelectionManager s_selectionManager;
	return s_selectionManager;
}
