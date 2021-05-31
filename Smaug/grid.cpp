#include "grid.h"
#include "svar.h"
#include "settingsmenu.h"
#include "smaugapp.h"
#include "svarex.h"
#include "basicdraw.h"
#include "utils.h"
#include "modelmanager.h"

#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/geometric.hpp>

BEGIN_SVAR_TABLE(CGridSettings)
	DEFINE_TABLE_SVAR_INPUT(increase, GLFW_KEY_RIGHT_BRACKET, false)
	DEFINE_TABLE_SVAR_INPUT(decrease, GLFW_KEY_LEFT_BRACKET, false)
END_SVAR_TABLE()

static CGridSettings s_gridSettings;
DEFINE_SETTINGS_MENU("Grid", s_gridSettings);



struct GridVertex
{
	float x;
	float y;
	float z;

	static void init()
	{
		ms_layout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.end();
	}
	static bgfx::VertexLayout ms_layout;
};
bgfx::VertexLayout GridVertex::ms_layout;

static GridVertex s_gridVertices[] =
{
	{-1.0f,  0.0f, -1.0f },
	{ 1.0f,  0.0f, -1.0f },
	{ 1.0f,  0.0f,  1.0f },
	{-1.0f,  0.0f,  1.0f },

};
static const uint16_t s_gridTriList[] = { 2, 1, 0, 0, 3, 2 };


CGrid::CGrid()
{
	m_iScale = 3;
	m_vecScale = glm::vec4(1 / (float)(pow(2, m_iScale)));;
	
	GridVertex::init();
	m_gridScale = bgfx::createUniform("gridScale", bgfx::UniformType::Vec4);
	m_gridDirMask = bgfx::createUniform("gridDirMask", bgfx::UniformType::Vec4);

	m_planeVertexBuf = bgfx::createVertexBuffer(bgfx::makeRef(s_gridVertices, sizeof(s_gridVertices)), GridVertex::ms_layout);
	m_planeIndexBuf = bgfx::createIndexBuffer(bgfx::makeRef(s_gridTriList, sizeof(s_gridTriList)));
}

void CGrid::Shutdown()
{
	bgfx::destroy(m_gridScale);
	bgfx::destroy(m_gridDirMask);
}


void CGrid::Update()
{
	// Clean this up!

	static int incLastCheck = -1;
	int inc = Input().IsDown(s_gridSettings.increase.GetValue());

	if (inc == GLFW_REPEAT || (incLastCheck == GLFW_PRESS && inc == GLFW_RELEASE))
	{
		if (m_iScale < 10)
		{
			m_iScale++;
			m_vecScale = glm::vec4(1 / (float)(pow(2, m_iScale)));
		}
		Log::Msg("[Grid] Scale: %d\n", m_iScale);
	}
	incLastCheck = inc;


	static int decLastCheck = -1;
	int dec = Input().IsDown(s_gridSettings.decrease.GetValue());

	if (dec == GLFW_REPEAT || (decLastCheck == GLFW_PRESS && dec == GLFW_RELEASE))
	{
		if (m_iScale > 1)
		{
			m_iScale--;
			m_vecScale = glm::vec4(1 / (float)(pow(2, m_iScale)));
		}
		Log::Msg("[Grid] Scale: %d\n", m_iScale);

	}
	decLastCheck = dec;

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

	glm::vec3 vecDir;
	Directions(angles, 0, 0, &vecDir);
	m_vecGridDirMask = { 1.0f - fabs(vecDir.x), 1.0f - fabs(vecDir.y), 1.0f - fabs(vecDir.z) };

	glm::vec3 scale = glm::vec3(screenSize.x, 0, screenSize.y);
	glm::mat4 mtx = glm::identity<glm::mat4>();
	mtx = glm::translate(mtx, pos + offsetHack);

	mtx *= glm::yawPitchRoll(angles.y, angles.x, angles.z);
	mtx = glm::scale(mtx, scale);



	bgfx::setTransform(&mtx[0][0]);
	bgfx::setVertexBuffer(0, m_planeVertexBuf);
	bgfx::setIndexBuffer(m_planeIndexBuf);

	bgfx::setUniform(m_gridScale, &m_vecScale);
	bgfx::setUniform(m_gridDirMask, &m_vecGridDirMask);
	bgfx::setState(
		BGFX_STATE_CULL_CW
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_DEPTH_TEST_ALWAYS
		| BGFX_STATE_MSAA
		| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA)
		| BGFX_STATE_BLEND_INDEPENDENT
		, BGFX_STATE_BLEND_FUNC_RT_1(BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_SRC_COLOR)
	);
	bgfx::submit(ModelManager().CurrentView(), ShaderManager().GetShaderProgram(Shader::GRID));
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
