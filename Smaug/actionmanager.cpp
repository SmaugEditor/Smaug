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

#if 0
	if (info.selected & ACT_SELECT_WALL)
		return info.node->Origin() + faceCenter(info.wall);
#endif

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

#if 0
	if (info.selected & ACT_SELECT_WALL)
	{
		glm::vec3 start = info.node->Origin() + (info.wall->bottomPoints[0] + info.wall->topPoints[0]) / 2.0f;
		glm::vec3 end = info.node->Origin() + (info.wall->bottomPoints[1] + info.wall->topPoints[1]) / 2.0f;
		float y = (( start + end ) / 2.0f).y;

		glm::vec2 newPos = SolveToLine2D({ pos.x, pos.z }, { start.x, start.z }, { end.x, end.z }, snap);
		return faceCenter(info.wall) + info.node->Origin();//{ newPos.x, y, newPos.y };
	}
#endif

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

bool CActionManager::FindFlags(glm::vec3 mousePos, selectionInfo_t& info, int findFlags, glm::vec3* outPointOfIntersect)
{


	// If this isn't 0, ACT_SELECT_NONE, we might have issues down the line
	info.selected = ACT_SELECT_NONE;

	// We successfully found nothing! Woo!
	if (findFlags == ACT_SELECT_NONE)
		return true;

	// What plane are we editing on?
	glm::vec3 workingAxis = GetCursor().GetWorkingAxis();
	glm::vec3 workingAxisMask = GetCursor().GetWorkingAxisMask();

	glm::vec3 pointOfIntersect = {0, 0, 0};

	// Find the selected item
	for (auto p : GetWorldEditor().m_nodes)
	{
		CNode* node = p.second;
		
		// Put the mouse on level with the node
		glm::vec3 localMouse = mousePos - node->Origin();
		localMouse *= workingAxisMask; // Flatten it out to just this plane
		aabb_t aabb = node->GetLocalAABB();
		localMouse += (aabb.max - aabb.min) * workingAxis / 2.0f; // Put our cursor in the middle

		// Check if we're in the AABB
		if (!testPointInAABB(localMouse, aabb, 1.5f))
			continue; // Not in bounds!
		
//		printf("IN BOUND");
		
					  
		// Do we want vertex selecting?
		if (findFlags & ACT_SELECT_VERT)
		{
			// Corner check
			//for (int j = 0; j < node->m_sideCount; j++)
			for (auto p : node->m_mesh.parts)
			{

				for (auto v : p->verts)
				{
					if (glm::distance(*v->vert, localMouse) <= 4
					 && glm::distance(*v->edge->vert->vert, localMouse) <= 4)
					{
						// Got a point. Add the flag and break the loop.
						info.selected |= ACT_SELECT_VERT;
						info.vertex = { v, node };
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

				testRayPlane_t t = pointOnPartLocal(p, localMouse);
				

				if (t.hit && glm::distance(t.intersect * workingAxisMask, localMouse * workingAxisMask) <= 1.5f)
				{
					// Discard parts with norms we don't like
					// TODO: Do this earlier! Ideally before the ray test!
					if (fabs(glm::dot(glm::normalize(t.normal), workingAxis)) > 0.89)
					{
						//printf("Skip\n");
						continue;
					}
					//printf("HIT\n");

					if (findFlags & ACT_SELECT_SIDE)
					{
						info.selected |= ACT_SELECT_SIDE;
						info.side = { p, node };
						
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
					{
						pointOfIntersect = t.intersect + node->Origin();
						break;
					}
				}

			}
		}

		// Did we find something?
		if (info.selected != ACT_SELECT_NONE)
		{
			// We almost always need the node anyway
			info.selected |= ACT_SELECT_NODE;
			info.node = node;

			if (outPointOfIntersect)
				*outPointOfIntersect = pointOfIntersect;

			// We got our selected item(s). Let's dip
			return true;
		}
		else if (findFlags & ACT_SELECT_NODE)
		{
			// They just want a node
			info.selected |= ACT_SELECT_NODE;
			info.node = node;

			if (outPointOfIntersect)
				*outPointOfIntersect = pointOfIntersect;

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
	int redoTo = INT_MAX;
	int undoTo = INT_MAX;

	if (ImGui::Begin("Edit History"))
	{
		if (m_redoStack.size())
		{
			for (int i = 0; i < m_redoStack.size(); i++)
			{
				ImGui::Text(m_redoStack[i]->GetName());
				if (ImGui::IsItemClicked())
				{
					redoTo = i;
				}
			}
			ImGui::Separator();
		}
		for (int i = m_actionHistory.size() - 1; i >= 0; i--)
		{
			ImGui::Text(m_actionHistory[i]->GetName());
			if (ImGui::IsItemClicked())
			{
				undoTo = i;
			}
		}
	}
	ImGui::End();

	if(m_actionHistory.size())
		for (int i = m_actionHistory.size() - 1; i >= undoTo; i--)
		{
			Undo();
		}
	
	if (m_redoStack.size())
		//for (int i = 0; i < redoTo+1; i++)
		for (int i = m_redoStack.size() - 1; i >= redoTo; i--)
		{
			Redo();
		}


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
