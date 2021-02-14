#include "cursor.h"
#include "basicdraw.h"
#include "modelmanager.h"

#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp> 
#include <imgui.h>

static glm::vec3 GetSelectionPos(selectionInfo_t info)
{
	// Smallest to largest
	if (info.selected & ACT_SELECT_VERT)
		return info.vertex->origin + info.node->m_origin;

	if (info.selected & ACT_SELECT_WALL)
		return info.node->m_origin + (info.wall->bottomPoints[0] + info.wall->bottomPoints[1] + info.wall->topPoints[0] + info.wall->topPoints[1]) / 4.0f;

	if (info.selected & ACT_SELECT_SIDE)
		return info.node->m_origin + (info.side->vertex1->origin + info.side->vertex2->origin) / 2.0f + glm::vec3(0, info.node->m_nodeHeight/2.0f, 0);

	if (info.selected & ACT_SELECT_NODE)
		return info.node->m_origin;

	return glm::vec3(0.0f, 0.0f, 0.0f);
}

CCursor::CCursor()
{
	m_position = glm::vec3(0,0,0);

	m_selectedObject = false;

	m_cursorSpinTime = 0;
	SetModel("assets/gizmo.obj");

}

void CCursor::Update(float dt)
{

	// Spin the cursor backwards if we're selecting something
	m_cursorSpinTime += dt * ( m_selectedObject ? 1 : -1 );
}

void CCursor::Draw()
{
	/*
	glm::mat4 mtx = glm::identity<glm::mat4>();
	mtx = glm::translate(mtx, m_position);
	mtx = glm::scale(mtx, glm::vec3(2.5f, 2.5f, 2.5f));
	mtx *= glm::yawPitchRoll(m_cursorSpinTime * 1.75f, m_cursorSpinTime * 0.5f, 0.0f);
	BasicDraw().Cube(mtx);
	*/
	
	m_model->Render(m_position, { m_cursorSpinTime * 1.75f, m_cursorSpinTime * 0.5f, 0.0f }, glm::vec3(2.5f, 2.5f, 2.5f));
}

void CCursor::SetPosition(glm::vec3 pos)
{
	m_selectedObject = false;
	m_position = pos;
}

void CCursor::SetSelection(selectionInfo_t info)
{
	m_selectedObject = info.selected != ACT_SELECT_NONE;

	if (m_selectedObject)
	{
		m_position = GetSelectionPos(info);
	}
}

void CCursor::SetModel(const char* path)
{
	m_model = ModelManager().LoadModel(path);
}

CCursor& GetCursor()
{
	static CCursor s_cursor;
	return s_cursor;
}
