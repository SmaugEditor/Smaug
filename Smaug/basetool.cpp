#include "basetool.h"
#include "actionmanager.h"
#include "editoractions.h"
#include "cursor.h"
#include "grid.h"
#include "selectedview.h"

#include <debugdraw.h>
#include <editorinterface.h>

#include <imgui.h>

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
		
		delta = Grid().Snap(delta);

		mousePosSnapped = delta + m_mouseStartDragPos;

		GetCursor().SetEditPosition(mousePosSnapped);
		m_mouseDragDelta = delta;

		if (!Input().IsDown({ MOUSE_1, true }))
		{
			EndDrag();
			SelectionManager().m_selected.clear();

			m_inDrag = false;
			m_selectionInfo.clear();

		}
		else
		{
			Preview();
		}
	}
	else
	{
		// If we're not currently, dragging, we want to watch for mouse input so we can begin our drag action
		if (!SelectionManager().BusySelecting() && Input().IsDown({ MOUSE_1, true }))
		{
			m_selectionInfo = SelectionManager().m_selected;

			m_inDrag = true;
			m_mouseStartDragPos = mousePosSnapped * GetCursor().GetWorkingAxisMask();
			m_mouseDragDelta = glm::vec3(0, 0, 0);

			SelectedView().SetSelection(m_selectionInfo.front().node);
			StartDrag();
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
	m_iconHandle = EngineInterface()->LoadTexture(GetIconPath());
}

void CBaseTool::Enable()
{
	GetCursor().SetModel(GetCursorPath());
}

void CBaseTool::ShowToolProperties()
{
	ImGui::Begin("Blank Tool", 0, ImGuiWindowFlags_NoDecoration);
	ImGui::End();
}
