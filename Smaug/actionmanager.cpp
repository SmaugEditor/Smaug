#include "actionmanager.h"
#include "smaugapp.h"
#include "utils.h"

CActionManager& GetActionManager()
{
	static CActionManager actionManager;
	
	return actionManager;
}

void CActionManager::Act(glm::vec3 mousePos)
{
	CSmaugApp& app = GetApp();

	if (actionMode == ActionMode::NONE)
	{
		// Should we pan the view?
		if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_3))
		{
			mouseStartPos = mousePos;
			actionMode = ActionMode::PAN_VIEW;
			return;
		}

		selectedWall = nullptr;
		selectedSide = nullptr;
		selectedVertex = nullptr;
		selectedNode = nullptr;
		bool hasFoundItem = false;
		cursorPos = mousePos;

		// Find the selected item
		for (int i = 0; i < GetWorldEditor().m_nodes.size(); i++)
		{
			CNode* node = GetWorldEditor().m_nodes[i];

			// Check if we're in the AABB
			// Note: Maybe add a threshold to this?

			if (!node->IsPointInAABB({ mousePos.x, mousePos.z }))
				continue; // Not in bounds!

			// Corner check
			for (int j = 0; j < node->m_sideCount; j++)
			{
				if (IsPointNearPoint2D(node->m_vertexes[j].origin + node->m_origin, mousePos, 4))
				{
					// Spin the cursor backwards if we're selecting something
					GetApp().m_uiView.m_editView.m_cursorSpin = -1;

					selectedVertex = &node->m_vertexes[j];
					selectedNode = node;

					cursorPos = selectedVertex->origin + node->m_origin;

					hasFoundItem = true;

					break;
				}
			}

			if (hasFoundItem)
				break;


			// Side's walls check
			for (int j = 0; j < node->m_sideCount; j++)
			{
				nodeSide_t* side = &node->m_sides[j];

				// Are we on the side?
				if (IsPointOnLine2D(side->vertex1->origin + node->m_origin, side->vertex2->origin + node->m_origin, mousePos, 2))
				{
					// Which wall are we selecting?
					for (int k = 0; k < side->walls.size(); k++)
					{
						nodeWall_t wall = side->walls[k];
						if (IsPointOnLine2D(wall.bottomPoints[0] + node->m_origin, wall.bottomPoints[1] + node->m_origin, mousePos, 2))
						{
							// Spin the cursor backwards if we're selecting something
							GetApp().m_uiView.m_editView.m_cursorSpin = -1;

							selectedWall = &side->walls[k];
							selectedSide = &node->m_sides[j];
							selectedNode = node;

							cursorPos = (wall.bottomPoints[0] + wall.bottomPoints[1] + wall.topPoints[0] + wall.topPoints[1]) / 4.0f + node->m_origin;

							hasFoundItem = true;
							printf("SELECTED WALL %f\n", selectedWall->bottomPoints[0].x);

							break;
						}
					}

					if (hasFoundItem)
						break;
				}
			}

			if (hasFoundItem)
				break;
		}

		if (hasFoundItem)
		{
			// Did we find a side?
			if (selectedWall)
			{

				// Should we extrude the selected side?
				if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_1) && app.isKeyDown(GLFW_KEY_LEFT_CONTROL))
				{
					printf("!SELECTED WALL %f\n", selectedWall->bottomPoints[0].x);
					mouseStartPos = mousePos;
					actionMode = ActionMode::EXTRUDE_SIDE;
					return;
				}
			}
			if (selectedSide)
			{
				// Should we extend the selected side?
				if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_1))
				{
					mouseStartPos = mousePos;
					actionMode = ActionMode::DRAG_SIDE;
					cursorPos = (selectedSide->vertex1->origin + selectedSide->vertex2->origin) / 2.0f + selectedNode->m_origin;
					return;
				}
			}
			if (selectedVertex)
			{

				// Should we drag the selected side?
				if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_1))
				{
					mouseStartPos = mousePos;
					actionMode = ActionMode::DRAG_VERTEX;
					return;
				}
			}
		}
	}
	else if (actionMode == ActionMode::PAN_VIEW)
	{
		if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_3))
		{
		}
		else
		{
			GetApp().m_uiView.m_editView.m_cameraPos += mousePos - mouseStartPos;
			mouseStartPos = mousePos;
			actionMode = ActionMode::NONE;
		}
	}
	else if (actionMode == ActionMode::DRAG_SIDE)
	{
		if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_1))
		{
		}
		else
		{
			glm::vec3 mouseDelta = mousePos - mouseStartPos;

			selectedSide->vertex1->origin += mouseDelta;
			selectedSide->vertex2->origin += mouseDelta;
			selectedNode->Update();

			printf("Stretched Node - Side\n");

			selectedNode = nullptr;
			selectedSide = nullptr;
			actionMode = ActionMode::NONE;
		}
	}
	else if (actionMode == ActionMode::DRAG_VERTEX)
	{
		if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_1))
		{
		}
		else
		{
			glm::vec3 mouseDelta = mousePos - mouseStartPos;

			selectedVertex->origin += mouseDelta;

			selectedNode->Update();

			printf("Stretched Node - Vertex\n");

			selectedNode = nullptr;
			selectedVertex = nullptr;
			actionMode = ActionMode::NONE;
		}
	}
	else if (actionMode == ActionMode::EXTRUDE_SIDE)
	{
		if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_1))
		{
		}
		else
		{
			printf("SELECTED WALL %f\n", selectedWall->bottomPoints[0].x);
			glm::vec3 mouseDelta = mousePos - mouseStartPos;

			CQuadNode* quad = GetWorldEditor().CreateQuad();
			quad->m_origin = selectedNode->m_origin;

			// If we don't flip vertex 1 and 2 here, the tri gets messed up and wont render.

			// 2 to 1 and 1 to 2
			// 
			// 2-----1
			// |     |
			// 1 --- 2

			nodeSide_t* attachedSide = &quad->m_sides[0];
			nodeSide_t* extrudedSide = &quad->m_sides[2];

			attachedSide->vertex1->origin = selectedWall->bottomPoints[1];
			attachedSide->vertex2->origin = selectedWall->bottomPoints[0];

			extrudedSide->vertex1->origin = selectedWall->bottomPoints[0] + mouseDelta;
			extrudedSide->vertex2->origin = selectedWall->bottomPoints[1] + mouseDelta;

			CConstraint sideConstraint;
			sideConstraint.SetParent(selectedNode, selectedSide);
			sideConstraint.SetChild(quad, attachedSide);
			quad->m_constrainedTo[quad->m_constrainedToCount] = sideConstraint;
			selectedNode->m_constraining.push_back(&quad->m_constrainedTo[quad->m_constrainedToCount]);
			quad->m_constrainedToCount++;

			quad->Update();


			// HACK HACK
			// For some reason we have to update the parent... Even though we just updated them...
			selectedNode->Update();

			printf("Created New Node\n");

			selectedNode = nullptr;
			selectedSide = nullptr;
			actionMode = ActionMode::NONE;

		}
	}
}
