#include "actionmanager.h"
#include "smaugapp.h"
#include "utils.h"
#include "cursor.h"
#include "raytest.h"


glm::vec3 GetSelectionPos(selectionInfo_t info)
{
	// Smallest to largest
	if (info.selected & ACT_SELECT_VERT)
		return info.node->Origin() + *info.vertex->vert;

	if (info.selected & ACT_SELECT_WALL)
		return info.node->Origin() + faceCenter(info.wall);

	if (info.selected & ACT_SELECT_SIDE)
		return info.node->Origin() + faceCenter(info.side);

	if (info.selected & ACT_SELECT_NODE)
		return info.node->Origin();

	return glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::vec3 SolvePosToSelection(selectionInfo_t info, glm::vec3 pos, SolveToLine2DSnap* snap)
{
	// Smallest to largest
	if (info.selected & ACT_SELECT_VERT)
		return *info.vertex->vert + info.node->Origin();

	if (info.selected & ACT_SELECT_WALL)
	{
#if 0
		glm::vec3 start = info.node->Origin() + (info.wall->bottomPoints[0] + info.wall->topPoints[0]) / 2.0f;
		glm::vec3 end = info.node->Origin() + (info.wall->bottomPoints[1] + info.wall->topPoints[1]) / 2.0f;
		float y = (( start + end ) / 2.0f).y;

		glm::vec2 newPos = SolveToLine2D({ pos.x, pos.z }, { start.x, start.z }, { end.x, end.z }, snap);
#endif
		return faceCenter(info.wall) + info.node->Origin();//{ newPos.x, y, newPos.y };
	}

	if (info.selected & ACT_SELECT_SIDE)
	{
#if 0
		glm::vec3 start = info.node->m_origin + info.side->vertex1->origin;
		glm::vec3 end = info.node->m_origin + info.side->vertex2->origin;
		float y = info.node->m_origin.y + info.node->m_nodeHeight / 2.0f;

		glm::vec2 newPos = SolveToLine2D({ pos.x, pos.z }, { start.x, start.z }, { end.x, end.z }, snap);
		return { newPos.x, y, newPos.y };
#endif
		return faceCenter(info.side) + info.node->Origin();
	}

	if (info.selected & ACT_SELECT_NODE)
		return info.node->Origin();

	return glm::vec3(0.0f, 0.0f, 0.0f);
}

CActionManager& GetActionManager()
{
	static CActionManager actionManager;
	
	return actionManager;
}


void CActionManager::CommitAction(IAction* action)
{
	action->Act();
	m_actionHistory.push_back(action);
	
	for (auto a : m_redoStack)
		delete a;
	m_redoStack.clear();
}

bool CActionManager::FindFlags(glm::vec3 mousePos, selectionInfo_t& info, int findFlags)
{
	// Flatten out our mousePos
	mousePos *= GetCursor().GetWorkingAxisMask();


	// If this isn't 0, ACT_SELECT_NONE, we might have issues down the line
	info.selected = ACT_SELECT_NONE;
	
	// We successfully found nothing! Woo!
	if (findFlags == ACT_SELECT_NONE)
		return true;

	// Find the selected item
	for (int i = 0; i < GetWorldEditor().m_nodes.size(); i++)
	{
		CNode* node = GetWorldEditor().m_nodes[i];

		// Check if we're in the AABB
		// Note: Maybe add a threshold to this?
		
		// Put the mouse within the node
		mousePos = mousePos * GetCursor().GetWorkingAxisMask() + node->Origin() * GetCursor().GetWorkingAxis();

		if (!node->IsPointInAABB(mousePos))
			continue; // Not in bounds!
		printf("IN BOUND");
		// Do we want vertex selecting?
		if (findFlags & ACT_SELECT_VERT)
		{
			// Corner check
			//for (int j = 0; j < node->m_sideCount; j++)
			for (auto p : node->m_mesh.parts)
			{

				for (auto v : p->verts)
				{
					if (glm::distance((node->Origin() + *v->vert) * GetCursor().GetWorkingAxisMask(), mousePos) <= 4
					 && glm::distance((node->Origin() + *v->edge->vert->vert) * GetCursor().GetWorkingAxisMask(), mousePos) <= 4)
					{
						// Got a point. Add the flag and break the loop.
						info.selected |= ACT_SELECT_VERT;
						info.vertex = v;
						break;
					}
				}
			}
		}

		if (findFlags & ACT_SELECT_WALL || findFlags & ACT_SELECT_SIDE)
		{
			bool foundSomething = false;
			// Side's walls check
			for (auto p : node->m_mesh.parts)
			{
				// Are we on the side?
				//mousePos.y = node->Origin().y;

				testRayPlane_t t = pointOnPart(p, mousePos);
				
				if (t.hit && glm::distance(t.intersect + node->Origin(), mousePos) <= 1.25f)
				{
					printf("HIT\n");

					if (findFlags & ACT_SELECT_SIDE)
					{
						info.selected |= ACT_SELECT_SIDE;
						info.side = p;
						
						foundSomething = true;
					}

					/*
					if (findFlags & ACT_SELECT_WALL)
					{
						// Which wall are we selecting?
						for(auto f : p->faces)
						{
							if (f->verts.size() == 3)
							{
								if (testPointInTri(mousePos, *f->verts[0]->vert, *f->verts[1]->vert, *f->verts[2]->vert))
								{

									info.selected |= ACT_SELECT_WALL;
									info.wall = f;

									// If they're asking for a wall, they're going to want a side too
									info.side = p;

									foundSomething = true;
									break;
								}
							}
						}
					}
					*/

					if (foundSomething)
						break;

				}

			}
		}

		// Did we find something?
		if (info.selected != ACT_SELECT_NONE)
		{
			// We almost always need the node anyway
			info.selected |= ACT_SELECT_NODE;
			info.node = node;

			// We got our selected item(s). Let's dip
			return true;
		}
		else if (findFlags & ACT_SELECT_NODE)
		{
			// They just want a node
			info.selected |= ACT_SELECT_NODE;
			info.node = node;

			return true;
		}
	}

	return false;
}

void CActionManager::Undo()
{
	if (m_actionHistory.size() == 0)
		return;

	IAction* a = m_actionHistory.back();
	m_actionHistory.pop_back();
	a->Undo();
	m_redoStack.push_back(a);
}

void CActionManager::Redo()
{
	if (m_redoStack.size() == 0)
		return;

	IAction* a = m_redoStack.back();
	m_redoStack.pop_back();
	a->Redo();
	m_actionHistory.push_back(a);
}

void CActionManager::Update()
{

	if (ImGui::Begin("Edit History"))
	{
		if (m_redoStack.size())
		{
			for (int i = m_redoStack.size() - 1; i >= 0; i--)
			{
				ImGui::Text(m_redoStack[i]->GetName());
			}
			ImGui::Separator();
		}
		for (int i = m_actionHistory.size() - 1; i >= 0; i--)
		{
			ImGui::Text(m_actionHistory[i]->GetName());
		}
	}
	ImGui::End();


	static bool keyUp = true;
	if (Input().IsDown({ GLFW_KEY_LEFT_CONTROL, false }) && Input().IsDown({ GLFW_KEY_Z, false }))
	{
		if(keyUp)
			Undo();
		keyUp = false;
	}
	else if (Input().IsDown({ GLFW_KEY_LEFT_CONTROL, false }) && Input().IsDown({ GLFW_KEY_Y, false }))
	{
		if (keyUp)
			Redo();
		keyUp = false;
	}
	else
	{
		keyUp = true;
	}
}
