#include "nodetools.h"
#include "svarex.h"
#include "settingsmenu.h"
#include "cursor.h"


BEGIN_SVAR_TABLE(CNodeToolsSettings)
	DEFINE_TABLE_SVAR_INPUT(dragToolToggle,    GLFW_KEY_G,          false)
	DEFINE_TABLE_SVAR_INPUT(dragToolHold,	   GLFW_KEY_UNKNOWN,    false)
	DEFINE_TABLE_SVAR_INPUT(extrudeToolToggle, GLFW_KEY_UNKNOWN,    false)
	DEFINE_TABLE_SVAR_INPUT(extrudeToolHold,   GLFW_KEY_LEFT_SHIFT, false)
	DEFINE_TABLE_SVAR_INPUT(paintToolToggle,   GLFW_KEY_UNKNOWN,    false)
	DEFINE_TABLE_SVAR_INPUT(paintToolHold,     GLFW_KEY_UNKNOWN,    false)
	DEFINE_TABLE_SVAR_INPUT(breakAlign,		   GLFW_KEY_LEFT_ALT,   false)
END_SVAR_TABLE()

static CNodeToolsSettings s_nodeToolsSettings;
DEFINE_SETTINGS_MENU("Node Tools", s_nodeToolsSettings);

input_t CDragTool::GetToggleInput() { return s_nodeToolsSettings.dragToolToggle; }
input_t CDragTool::GetHoldInput() { return s_nodeToolsSettings.dragToolHold; }

input_t CExtrudeTool::GetToggleInput() { return s_nodeToolsSettings.extrudeToolToggle; }
input_t CExtrudeTool::GetHoldInput() { return s_nodeToolsSettings.extrudeToolHold; }

input_t CPaintTool::GetToggleInput() { return s_nodeToolsSettings.paintToolToggle; }
input_t CPaintTool::GetHoldInput() { return s_nodeToolsSettings.paintToolHold; }


void CDragTool::StartDrag()
{
	m_action = new CDragAction;

	m_action->SetMoveStart(m_mouseStartDragPos);
	m_action->Select(m_selectionInfo);
}

void CDragTool::EndDrag()
{
	// If we've got just one item, lock it to the axis of the norm
	if (m_selectionInfo.size() == 1 && !Input().IsDown(s_nodeToolsSettings.breakAlign))
	{
		auto& si = m_selectionInfo[0];
		if (si.part.IsValid())
			m_mouseDragDelta = si.part->normal * glm::dot(m_mouseDragDelta, si.part->normal);
	}

	if (glm::length(m_mouseDragDelta) == 0)
	{
		// No delta... Delete the action and move on
		delete m_action;
		m_action = nullptr;
	}
	else
	{
		m_action->SetMoveDelta(m_mouseDragDelta);
		GetActionManager().CommitAction(m_action);

		// We don't need to delete the action
		// The manager is taking care of it for us now
		m_action = nullptr;
	}
};

void CDragTool::Preview()
{
	// If we've got just one item, lock it to the axis of the norm
	if (m_selectionInfo.size() == 1 && !Input().IsDown(s_nodeToolsSettings.breakAlign))
	{
		auto& si = m_selectionInfo[0];
		if (si.part.IsValid())
		{
			m_mouseDragDelta = si.part->normal * glm::dot(m_mouseDragDelta, si.part->normal);
		}
	}

	if (m_action)
	{
		m_action->SetMoveDelta(m_mouseDragDelta);
		m_action->Preview();
	}
}

void CExtrudeTool::StartDrag()
{
	m_wallExtrudeAction = new CWallExtrudeAction;
	m_wallExtrudeAction->Select(m_selectionInfo);
	m_wallExtrudeAction->SetMoveStart(m_mouseStartDragPos);
}

void CExtrudeTool::EndDrag()
{
	// If we've got just one item, lock it to the axis of the norm
	if (m_selectionInfo.size() == 1 && !Input().IsDown(s_nodeToolsSettings.breakAlign))
	{
		auto& si = m_selectionInfo[0];
		if (si.part.IsValid())
		{
			m_mouseDragDelta = si.part->normal * glm::dot(m_mouseDragDelta, si.part->normal);
		}
	}

	if (glm::length(m_mouseDragDelta) == 0)
	{
		// No delta... Delete the action and move on
		delete m_wallExtrudeAction;
		m_wallExtrudeAction = nullptr;
	}
	else
	{
		m_wallExtrudeAction->SetMoveDelta(m_mouseDragDelta);
		GetActionManager().CommitAction(m_wallExtrudeAction);
		// We don't need to delete the action
		// The manager is taking care of it for us now
		m_wallExtrudeAction = nullptr;
	}

};

void CExtrudeTool::Preview()
{
	// If we've got just one item, lock it to the axis of the norm
	if (m_selectionInfo.size() == 1 && !Input().IsDown(s_nodeToolsSettings.breakAlign))
	{
		auto& si = m_selectionInfo[0];
		if (si.part.IsValid())
		{
			m_mouseDragDelta = si.part->normal * glm::dot(m_mouseDragDelta, si.part->normal);
		}
	}

	if (m_wallExtrudeAction)
	{
		m_wallExtrudeAction->SetMoveDelta(m_mouseDragDelta);
		m_wallExtrudeAction->Preview();
	}
}


void CPaintTool::Update(float dt, glm::vec3 mousePosSnapped, glm::vec3 mousePosRaw)
{
	GetCursor().SetPosition(mousePosRaw);
	if (!SelectionManager().BusySelecting() && Input().Clicked({GLFW_MOUSE_BUTTON_1, true}))
	{
		if(!m_paintAction)
			m_paintAction = new CPaintAction();

		m_paintAction->Select(SelectionManager().m_selected);
		m_paintAction->m_newPaint = { {0, 0}, {0.125, 0.125}, m_browser.SelectedTexture() };
		GetActionManager().CommitAction(m_paintAction);
		m_paintAction = nullptr;
		SelectionManager().m_selected.clear();
	}
	
}

void CPaintTool::ShowToolProperties()
{
	m_browser.Show();
}
