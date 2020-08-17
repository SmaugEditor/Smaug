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


			// Side check
			for (int j = 0; j < node->m_sideCount; j++)
			{
				if (IsPointOnLine2D(node->m_sides[j].vertex1->origin + node->m_origin, node->m_sides[j].vertex2->origin + node->m_origin, mousePos, 2))
				{
					// Spin the cursor backwards if we're selecting something
					GetApp().m_uiView.m_editView.m_cursorSpin = -1;

					selectedSide = &node->m_sides[j];
					selectedNode = node;

					cursorPos = (selectedSide->vertex1->origin + selectedSide->vertex2->origin) / 2.0f + node->m_origin;

					hasFoundItem = true;

					break;
				}
			}

			if (hasFoundItem)
				break;
		}


		// Did we find a side?
		if (selectedSide)
		{
			// Should we extrude the selected side?
			if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_1) && app.isKeyDown(GLFW_KEY_LEFT_CONTROL))
			{
				mouseStartPos = mousePos;
				actionMode = ActionMode::EXTRUDE_SIDE;
			}
			// Should we extend the selected side?
			else if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_1))
			{
				mouseStartPos = mousePos;
				actionMode = ActionMode::DRAG_SIDE;
			}

		}
		else if (selectedVertex)
		{

			// Should we drag the selected side?
			if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_1))
			{
				mouseStartPos = mousePos;
				actionMode = ActionMode::DRAG_VERTEX;
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

			attachedSide->vertex1->origin = selectedSide->vertex2->origin;
			attachedSide->vertex2->origin = selectedSide->vertex1->origin;

			extrudedSide->vertex1->origin = selectedSide->vertex1->origin + mouseDelta;
			extrudedSide->vertex2->origin = selectedSide->vertex2->origin + mouseDelta;

			quad->Update();

			CConstraint vert1Cons;
			vert1Cons.SetParent(selectedNode, selectedSide);
			vert1Cons.SetChild(quad, attachedSide->vertex1);
			quad->m_constrainedTo[quad->m_constrainedToCount] = vert1Cons;
			selectedNode->m_constraining.push_back(&quad->m_constrainedTo[quad->m_constrainedToCount]);
			quad->m_constrainedToCount++;

			CConstraint vert2Cons;
			vert2Cons.SetParent(selectedNode, selectedSide);
			vert2Cons.SetChild(quad, attachedSide->vertex2);
			quad->m_constrainedTo[quad->m_constrainedToCount] = vert2Cons;
			selectedNode->m_constraining.push_back(&quad->m_constrainedTo[quad->m_constrainedToCount]);
			quad->m_constrainedToCount++;


			printf("Created New Node\n");

			selectedNode = nullptr;
			selectedSide = nullptr;
			actionMode = ActionMode::NONE;

		}
	}
}
