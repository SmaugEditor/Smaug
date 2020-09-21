#pragma once

#include "basetool.h"
#include "editoractions.h"

#include <GLFW/glfw3.h>

class CDragTool : public CBaseDragTool
{
public:

	virtual const char* GetName() { return "Drag"; }
	virtual const char* GetIconPath() { return "drag.png"; }

	// When this key is pressed, this tool becomes active
	virtual int GetToggleKey() { return GLFW_KEY_G; }

	// While this key is held, this tool is active
	virtual int GetHoldKey() { return 0; }


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
	virtual const char* GetIconPath() { return "extrude.png"; }

	// When this key is pressed, this tool becomes active
	virtual int GetToggleKey() { return 0; }

	// While this key is held, this tool is active
	virtual int GetHoldKey() { return GLFW_KEY_LEFT_SHIFT; }


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
