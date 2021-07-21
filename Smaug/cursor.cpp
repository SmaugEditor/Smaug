#include "cursor.h"
#include "basicdraw.h"
#include "modelmanager.h"
#include "grid.h"
#include "utils.h"

#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp> 
#include <imgui.h>


CCursor::CCursor()
{
	m_position = glm::vec3(0,0,0);

	m_cursorMode = CursorMode::NONE;

	m_cursorSpinTime = 0;
	SetModel("assets/gizmo.obj");

}

void CCursor::Update(float dt)
{

	// Spin the cursor backwards if we're selecting something
	m_cursorSpinTime += dt * ( m_cursorMode != CursorMode::EDITING ? 1 : -1 );
}

void CCursor::Draw(float scale)
{
	glm::vec3 rot = glm::vec3(0);
	glm::vec3 pos = m_position;
	if (m_cursorMode != CursorMode::NONE)
	{
		rot = { m_cursorSpinTime * 1.75f, m_cursorSpinTime * 0.5f, 0.0f };
	}
	else
	{
		//pos = Grid().Snap(m_position);
	}

	m_model->Render(pos, rot, glm::vec3(scale));
}

void CCursor::SetPosition(glm::vec3 pos)
{
	SetMode(CursorMode::NONE);
	m_position = pos;
}

glm::vec3 CCursor::GetPosition()
{
	return Grid().Snap(m_position);
}

glm::vec3 CCursor::SetSelection(glm::vec3 pos, selectionInfo_t info)
{
	/*
	if (info.selected != ACT_SELECT_NONE)
	{
		SetMode(CursorMode::HOVERING);
		
		// TODO: Clean out the double pass!
		SolveToLine2DSnap snap = SolveToLine2DSnap::POINT;
		m_position = SolvePosToSelection(info, pos, &snap);
		glm::vec3 snapedPos = Grid().Snap(m_position);
		switch (snap)
		{
		case SolveToLine2DSnap::POINT:
			break;
		case SolveToLine2DSnap::Y_SNAP:
			m_position = { snapedPos.x, m_position.y, snapedPos.z };
			m_position = SolvePosToSelection(info, m_position);
			break;
		case SolveToLine2DSnap::X_SNAP:
			m_position = { snapedPos.x, m_position.y, snapedPos.z };
			m_position = SolvePosToSelection(info, m_position);
			break;
		default:
			break;
		}

		return m_position;
	}
	else if (m_cursorMode == CursorMode::HOVERING)
	{
		SetMode(CursorMode::NONE);
		return pos;
	}
	*/
	return pos;
}

void CCursor::SetEditPosition(glm::vec3 pos)
{
	SetMode(CursorMode::EDITING);
	m_position = pos;
}

void CCursor::SetModel(const char* path)
{
	m_model = ModelManager().LoadModel(path);
}

void CCursor::SetMode(CursorMode mode)
{
	// If we're moving from none, we need to start the spin
	if (m_cursorMode == CursorMode::NONE)
		m_cursorSpinTime = 0;

	m_cursorMode = mode;
}

void CCursor::SetWorkingAxis(glm::vec3 vecAxis)
{
	m_workingAxis = vecAxis;
	m_workingAxisMask = dirMask(vecAxis);
}

CCursor& GetCursor()
{
	static CCursor s_cursor;
	return s_cursor;
}
