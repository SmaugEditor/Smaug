#include "basetool.h"
#include "actionmanager.h"
#include "editoractions.h"

void CDragTool::Enable()
{
	GetActionManager().m_selectedAction = (IAction*)new CVertDragAction;
}

void CDragTool::Disable()
{
	delete GetActionManager().m_selectedAction;
	GetActionManager().m_selectedAction = nullptr;
}

void CExtrudeTool::Enable()
{
	GetActionManager().m_selectedAction = (IAction*)new CWallExtrudeAction;
}

void CExtrudeTool::Disable()
{
	delete GetActionManager().m_selectedAction;
	GetActionManager().m_selectedAction = nullptr;
}
