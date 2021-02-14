#include "basetool.h"
#include "actionmanager.h"
#include "editoractions.h"
#include "smaugapp.h"
#include "cursor.h"
#include "texturemanager.h"

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
		if (fabs(delta.x) < 2)
			delta.x = 0;
		else if (fabs(delta.z) < 2)
			delta.z = 0;
		delta.y = 0;

		mousePosSnapped = delta + m_mouseStartDragPos;

		GetCursor().SetEditPosition(mousePosSnapped);

		// Using glfw input until something is figured out about ImGui's overriding
		if (glfwGetMouseButton(GetApp().GetWindow(), GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
		{
			m_mouseDragDelta = delta;
			EndDrag();

			m_inDrag = false;
			m_selectionInfo.selected = ACT_SELECT_NONE;
		}
	}
	else
	{
		// If we're not currently, dragging, we want to watch for mouse input so we can begin our drag action

		// Check if we're hovering on anything
		selectionInfo_t info;
		bool found = GetActionManager().FindFlags(mousePosRaw, info, GetSelectionType());
		
		if (found && info.selected != ACT_SELECT_NONE)
		{
			// Got something! We can use this for our cursor if we have something
			glm::vec3 solvedPos = GetCursor().SetSelection(mousePosRaw, info);
			

			// If we found something, and we're clicking, we can start dragging!
			// Using glfw input until something is figured out about ImGui's overriding
			if (glfwGetMouseButton(GetApp().GetWindow(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
			{
				m_selectionInfo = info;
				m_inDrag = true;
				m_mouseStartDragPos = solvedPos;// GetSelectionPos(info);
				m_mouseDragDelta = glm::vec3(0, 0, 0);

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
