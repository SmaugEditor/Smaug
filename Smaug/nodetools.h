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
	virtual input_t GetToggleInput();

	// While this key is held, this tool is active
	virtual input_t GetHoldInput();


	virtual SelectionFlag GetSelectionType() { return (SelectionFlag)(SelectionFlag::SF_PART /*| SelectionFlag::SF_EDGE*/ | SelectionFlag::SF_NODE); }

	virtual void StartDrag();
	virtual void EndDrag();

	virtual void Preview();

	CBaseDragAction* m_action;
};


class CExtrudeTool : public CBaseDragTool
{
public:

	virtual const char* GetName() { return "Extrude"; }
	virtual const char* GetIconPath() { return "assets/extrude.png"; }
	virtual const char* GetCursorPath() { return "assets/extend_gizmo.obj"; }

	// When this key is pressed, this tool becomes active
	virtual input_t GetToggleInput();

	// While this key is held, this tool is active
	virtual input_t GetHoldInput();


	virtual SelectionFlag GetSelectionType() { return SelectionFlag::SF_PART; }

	virtual void StartDrag();
	virtual void EndDrag();

	virtual void Preview();

	CWallExtrudeAction* m_wallExtrudeAction;
};
