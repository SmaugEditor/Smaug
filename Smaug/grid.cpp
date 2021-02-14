#include "grid.h"
#include "svar.h"
#include "settingsmenu.h"
#include "smaugapp.h"
#include "svarex.h"
#include "basicdraw.h"

#include <GLFW/glfw3.h>
#include <modelmanager.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/geometric.hpp>

BEGIN_SVAR_TABLE(CGridSettings)
	DEFINE_TABLE_SVAR_KEY(increase, GLFW_KEY_RIGHT_BRACKET)
	DEFINE_TABLE_SVAR_KEY(decrease, GLFW_KEY_LEFT_BRACKET)
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
	{ 0.0f,  0.0f,  0.0f },
	{ 1.0f,  0.0f,  0.0f },
	{ 1.0f,  0.0f,  1.0f },
	{ 0.0f,  0.0f,  1.0f },

};
static const uint16_t s_gridTriList[] = { 2, 1, 0, 0, 3, 2 };


CGrid::CGrid()
{
	m_iScale = 2;
	m_vecScale = glm::vec4(1);
	
	GridVertex::init();
	m_gridScale = bgfx::createUniform("s_gridScale", bgfx::UniformType::Vec4);
	
	m_planeVertexBuf = bgfx::createVertexBuffer(bgfx::makeRef(s_gridVertices, sizeof(s_gridVertices)), GridVertex::ms_layout);
	m_planeIndexBuf = bgfx::createIndexBuffer(bgfx::makeRef(s_gridTriList, sizeof(s_gridTriList)));
}

void CGrid::Shutdown()
{
	bgfx::destroy(m_gridScale);
}


void CGrid::Update()
{
	// Clean this up!

	static int incLastCheck = -1;
	int inc = glfwGetKey(GetApp().GetWindow(), s_gridSettings.increase.GetValue().key);

	if (inc == GLFW_REPEAT || (incLastCheck == GLFW_PRESS && inc == GLFW_RELEASE))
	{
		if (m_iScale < 10)
		{
			m_iScale++;
			m_vecScale = glm::vec4(1 / (float)(pow(2, m_iScale)));
		}
		printf("[Grid] Scale: %d\n", m_iScale);
	}
	incLastCheck = inc;


	static int decLastCheck = -1;
	int dec = glfwGetKey(GetApp().GetWindow(), s_gridSettings.decrease.GetValue().key);

	if (dec == GLFW_REPEAT || (decLastCheck == GLFW_PRESS && dec == GLFW_RELEASE))
	{
		if (m_iScale > 1)
		{
			m_iScale--;
			m_vecScale = glm::vec4(1 / (float)(pow(2, m_iScale)));
		}
		printf("[Grid] Scale: %d\n", m_iScale);

	}
	decLastCheck = dec;

}

void CGrid::Draw()
{
	glm::vec3 scale = glm::vec3(m_screen.y - m_screen.x, 0, m_screen.w - m_screen.z);
	glm::mat4 mtx = glm::identity<glm::mat4>();
	mtx = glm::translate(mtx, glm::vec3(m_screen.x, -0.01, m_screen.z));
	mtx = glm::scale(mtx, scale);

	bgfx::setTransform(&mtx[0][0]);
	bgfx::setVertexBuffer(0, m_planeVertexBuf);
	bgfx::setIndexBuffer(m_planeIndexBuf);

	bgfx::setUniform(m_gridScale, &m_vecScale);

	bgfx::submit(ModelManager().CurrentView(), ShaderManager::GetShaderProgram(Shader::GRID));
}

void CGrid::Draw(glm::vec4 screen)
{
	m_screen = screen;
	Draw();
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
