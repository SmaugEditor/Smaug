#pragma once

#include "basetool.h"
#include "editoractions.h"

#include <GLFW/glfw3.h>

class CDragTool : public CBaseDragTool
{
public:

	virtual const char* GetName() { return "Drag"; }
	virtual const char* GetIconPath() { return "assets/drag.png"; }
	virtual const char* GetCursorPath() { return "assets/extrude_gizmo.obj"; }

	// When this key is pressed, this tool becomes active
	virtual int GetToggleKey();

	// While this key is held, this tool is active
	virtual int GetHoldKey();


	virtual int GetSelectionType() { return ACT_SELECT_SIDE | ACT_SELECT_VERT; }

	virtual void StartDrag()
	{
		if (m_selectionInfo.selected & ACT_SELECT_VERT)
		{
			m_action = new CVertDragAction;
		}
		else
		{
			m_action = new CSideDragAction;
		}

		m_action->Select(m_selectionInfo);
	}

	virtual void EndDrag()
	{
		m_action->SetMoveDelta(m_mouseDragDelta);
		GetActionManager().CommitAction(m_action);

		// We don't need to delete the action
		// The manager is taking care of it for us now
		m_action = nullptr;
	};

	CBaseDragAction* m_action;
};


class CExtrudeTool : public CBaseDragTool
{
public:

	virtual const char* GetName() { return "Extrude"; }
	virtual const char* GetIconPath() { return "assets/extrude.png"; }
	virtual const char* GetCursorPath() { return "assets/extend_gizmo.obj"; }

	// When this key is pressed, this tool becomes active
	virtual int GetToggleKey();

	// While this key is held, this tool is active
	virtual int GetHoldKey();


	virtual int GetSelectionType() { return ACT_SELECT_WALL; }

	virtual void StartDrag()
	{
	}

	virtual void EndDrag()
	{
		m_wallExtrudeAction = new CWallExtrudeAction;
		m_wallExtrudeAction->Select(m_selectionInfo);
		m_wallExtrudeAction->SetMoveDelta(m_mouseDragDelta);
		GetActionManager().CommitAction(m_wallExtrudeAction);

		// We don't need to delete the action
		// The manager is taking care of it for us now
		m_wallExtrudeAction = nullptr;
	};

	CWallExtrudeAction* m_wallExtrudeAction;
};
