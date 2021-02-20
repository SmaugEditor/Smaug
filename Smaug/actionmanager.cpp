#include "actionmanager.h"
#include "smaugapp.h"
#include "utils.h"
#include "cursor.h"


glm::vec3 GetSelectionPos(selectionInfo_t info)
{
	// Smallest to largest
	if (info.selected & ACT_SELECT_VERT)
		return info.vertex->origin + info.node->m_origin;

	if (info.selected & ACT_SELECT_WALL)
		return info.node->m_origin + (info.wall->bottomPoints[0] + info.wall->bottomPoints[1] + info.wall->topPoints[0] + info.wall->topPoints[1]) / 4.0f;

	if (info.selected & ACT_SELECT_SIDE)
		return info.node->m_origin + (info.side->vertex1->origin + info.side->vertex2->origin) / 2.0f + glm::vec3(0, info.node->m_nodeHeight / 2.0f, 0);

	if (info.selected & ACT_SELECT_NODE)
		return info.node->m_origin;

	return glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::vec3 SolvePosToSelection(selectionInfo_t info, glm::vec3 pos, SolveToLine2DSnap* snap)
{
	// Smallest to largest
	if (info.selected & ACT_SELECT_VERT)
		return info.vertex->origin + info.node->m_origin + glm::vec3(0, info.node->m_nodeHeight / 2.0f, 0);

	if (info.selected & ACT_SELECT_WALL)
	{
		glm::vec3 start = info.node->m_origin + (info.wall->bottomPoints[0] + info.wall->topPoints[0]) / 2.0f;
		glm::vec3 end = info.node->m_origin + (info.wall->bottomPoints[1] + info.wall->topPoints[1]) / 2.0f;
		float y = (( start + end ) / 2.0f).y;

		glm::vec2 newPos = SolveToLine2D({ pos.x, pos.z }, { start.x, start.z }, { end.x, end.z }, snap);
		return { newPos.x, y, newPos.y };
	}

	if (info.selected & ACT_SELECT_SIDE)
	{
		glm::vec3 start = info.node->m_origin + info.side->vertex1->origin;
		glm::vec3 end = info.node->m_origin + info.side->vertex2->origin;
		float y = info.node->m_origin.y + info.node->m_nodeHeight / 2.0f;

		glm::vec2 newPos = SolveToLine2D({ pos.x, pos.z }, { start.x, start.z }, { end.x, end.z }, snap);
		return { newPos.x, y, newPos.y };
	}

	if (info.selected & ACT_SELECT_NODE)
		return info.node->m_origin;

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
}

bool CActionManager::FindFlags(glm::vec3 mousePos, selectionInfo_t& info, int findFlags)
{
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

		if (!node->IsPointInAABB(mousePos))
			continue; // Not in bounds!


		// Do we want vertex selecting?
		if (findFlags & ACT_SELECT_VERT)
		{
			// Corner check
			for (int j = 0; j < node->m_sideCount; j++)
			{
				if (IsPointNearPoint2D(node->m_vertexes[j].origin + node->m_origin, mousePos, 4))
				{
					// Got a point. Add the flag and break the loop.
					info.selected |= ACT_SELECT_VERT;
					info.vertex = &node->m_vertexes[j];
					break;
				}
			}
		}

		if (findFlags & ACT_SELECT_WALL || findFlags & ACT_SELECT_SIDE)
		{
			bool foundSomething = false;
			// Side's walls check
			for (int j = 0; j < node->m_sideCount; j++)
			{
				nodeSide_t* side = &node->m_sides[j];

				// Are we on the side?
				if (IsPointOnLine2D(side->vertex1->origin + node->m_origin, side->vertex2->origin + node->m_origin, mousePos, 2))
				{
					if (findFlags & ACT_SELECT_SIDE)
					{
						info.selected |= ACT_SELECT_SIDE;
						info.side = &node->m_sides[j];
						
						foundSomething = true;
					}


					if (findFlags & ACT_SELECT_WALL)
					{
						// Which wall are we selecting?
						for (int k = 0; k < side->walls.size(); k++)
						{
							nodeWall_t wall = side->walls[k];
							if (IsPointOnLine2D(wall.bottomPoints[0] + node->m_origin, wall.bottomPoints[1] + node->m_origin, mousePos, 2))
							{
								info.selected |= ACT_SELECT_WALL;
								info.wall = &side->walls[k];

								// If they're asking for a wall, they're going to want a side too
								info.side = &node->m_sides[j];

								foundSomething = true;
								break;
							}
						}
					}

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
