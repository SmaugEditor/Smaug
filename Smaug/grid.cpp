#include "grid.h"
#include "svar.h"
#include "settingsmenu.h"
#include "svarex.h"
#include "utils.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/geometric.hpp>
#include <transform.h>
#include <editorinterface.h>

BEGIN_SVAR_TABLE(CGridSettings)
	DEFINE_TABLE_SVAR_INPUT(increase, KEY_RIGHT_BRACKET, false)
	DEFINE_TABLE_SVAR_INPUT(decrease, KEY_LEFT_BRACKET, false)
END_SVAR_TABLE()

static CGridSettings s_gridSettings;
DEFINE_SETTINGS_MENU("Grid", s_gridSettings);



CGrid::CGrid()
{
	m_iScale = 3;
	m_vecScale = glm::vec4(1 / (float)(pow(2, m_iScale)));;
	
}

void CGrid::Shutdown()
{
}


void CGrid::Update()
{

	if (Input().Clicked(s_gridSettings.increase.GetValue()))
	{
		if (m_iScale < 10)
		{
			m_iScale++;
			m_vecScale = glm::vec4(1 / (float)(pow(2, m_iScale)));
		}
		Log::Msg("[Grid] Scale: %d\n", m_iScale);
	}

	if (Input().Clicked(s_gridSettings.decrease.GetValue()))
	{
		if (m_iScale > 1)
		{
			m_iScale--;
			m_vecScale = glm::vec4(1 / (float)(pow(2, m_iScale)));
		}
		Log::Msg("[Grid] Scale: %d\n", m_iScale);

	}
}

void CGrid::Draw()
{
	Draw(m_screenSize, m_pos, m_angles, false);
}

void CGrid::Draw(glm::vec2 screenSize, glm::vec3 pos, glm::vec3 angles, bool submitTo3D, glm::vec3 offsetHack)
{
	if (submitTo3D)
	{
		m_angles = angles;
		m_screenSize = screenSize;
		m_pos = pos;
	}

	CTransform t;
	t.SetLocalScale(screenSize.x, 0, screenSize.y);
	t.SetLocalAngles(angles);
	t.SetLocalOrigin(pos + offsetHack);
	
	EngineInterface()->DrawGrid(t, m_iScale);

}

glm::vec3 CGrid::Snap(glm::vec3 in)
{
	return { round(in.x * m_vecScale.x * 2.0f) / m_vecScale.x / 2.0f, round(in.y * m_vecScale.y * 2.0f) / m_vecScale.y / 2.0f, round(in.z * m_vecScale.z * 2.0f) / m_vecScale.z / 2.0f };
}


CGrid& Grid()
{
	static CGrid s_grid;
	return s_grid;
}
