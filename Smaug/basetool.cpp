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

void CBaseDragTool::Update(float dt, glm::vec3 mousePos)
{
	if (m_inDrag)
	{
		// When we're in drag, we're waiting for the mouse to be released so we can fire off our action

		// Using glfw input until something is figured out about ImGui's overriding
		if (glfwGetMouseButton(GetApp().GetWindow(), GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
		{
			m_mouseDragDelta = mousePos - m_mouseStartDragPos;
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
		bool found = GetActionManager().FindFlags(mousePos, info, GetSelectionType());
		
		if(found && info.selected != ACT_SELECT_NONE)
		{
			// Got something! We can use this for our cursor if we have something
			GetCursor().SetSelection(info);
		}
		else
		{
			// Can't use our search for our mouse pos :(
			GetCursor().SetPosition(mousePos);
		}

		// If we found something, and we're clicking, we can start dragging!
		// Using glfw input until something is figured out about ImGui's overriding
		if (found && glfwGetMouseButton(GetApp().GetWindow(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
		{
			m_selectionInfo = info;
			m_inDrag = true;
			m_mouseStartDragPos = mousePos;
			m_mouseDragDelta = glm::vec3(0, 0, 0);

			StartDrag();
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
