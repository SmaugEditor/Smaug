#include "basetool.h"
#include "actionmanager.h"
#include "editoractions.h"
#include "smaugapp.h"
#include "cursor.h"
#include "texturemanager.h"
#include "grid.h"

#include <GLFW/glfw3.h>


void CBaseDragTool::Enable()
{
	CBaseTool::Enable();
	m_inDrag = false;

}

void CBaseDragTool::Update(float dt, glm::vec3 mousePosSnapped, glm::vec3 mousePosRaw)
{
	if (m_inDrag)
	{
		// When we're in drag, we're waiting for the mouse to be released so we can fire off our action
		

		glm::vec3 delta = mousePosSnapped - m_mouseStartDragPos;
		
		// TODO: Improve for when zooming and etc!
		// Try to keep things on one axis a bit
		/*
		if (fabs(delta.x) < 2)
			delta.x = 0;
		else if (fabs(delta.z) < 2)
			delta.z = 0;
		else if (fabs(delta.y) < 2)
			delta.y = 0;
		*/
		
		delta *= GetCursor().GetWorkingAxisMask();
		if (!Input().IsDown({ GLFW_KEY_LEFT_ALT, false }) && m_selectionInfo.side.IsValid())
		{
			glm::vec3 fn = glm::normalize(faceNormal(m_selectionInfo.side));
			delta = fn * glm::dot(delta, fn);
		}
		else
		{
			glm::vec3 fn = glm::normalize(glm::cross(faceNormal(m_selectionInfo.side), GetCursor().GetWorkingAxis()));
			delta = fn * glm::dot(delta, fn);
		}

		delta = Grid().Snap(delta);

		mousePosSnapped = delta + m_mouseStartDragPos;

		GetCursor().SetEditPosition(mousePosSnapped);
		m_mouseDragDelta = delta;

		// Using glfw input until something is figured out about ImGui's overriding
		if (glfwGetMouseButton(GetApp().GetWindow(), GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
		{
			EndDrag();

			m_inDrag = false;
			m_selectionInfo.selected = ACT_SELECT_NONE;
		}
		else
		{
			Preview();
		}
	}
	else
	{
		// If we're not currently, dragging, we want to watch for mouse input so we can begin our drag action

		// Check if we're hovering on anything
		selectionInfo_t info;
		glm::vec3 poi = {0,0,0};
		bool found = GetActionManager().FindFlags(mousePosRaw, info, GetSelectionType(), &poi);
		
		if (found && info.selected != ACT_SELECT_NONE)
		{
			// Got something! We can use this for our cursor if we have something
			// Move this all back into the cursor!
			glm::vec3 solvedPos = poi;// GetCursor().SetSelection(mousePosRaw, info);
			solvedPos = Grid().Snap(solvedPos);
			GetCursor().SetMode(CursorMode::HOVERING);
			GetCursor().SetPositionForce(solvedPos);

			// If we found something, and we're clicking, we can start dragging!
			// Using glfw input until something is figured out about ImGui's overriding
			if (glfwGetMouseButton(GetApp().GetWindow(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
			{
				m_selectionInfo = info;
				m_inDrag = true;
				m_mouseStartDragPos = solvedPos * GetCursor().GetWorkingAxisMask();// GetSelectionPos(info);
				m_mouseDragDelta = glm::vec3(0, 0, 0);

				SelectedView().SetSelection(info.node);
				StartDrag();
			}
		}
		else
		{
			// Can't use our search for our mouse pos :(
			GetCursor().SetPosition(mousePosRaw);

		}
	}
}

void CBaseTool::Init()
{
	m_iconHandle = TextureManager().LoadTexture(GetIconPath());
}

void CBaseTool::Enable()
{
	GetCursor().SetModel(GetCursorPath());
}
