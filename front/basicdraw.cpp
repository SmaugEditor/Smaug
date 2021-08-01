#include "basicdraw.h"
#include "modelmanager.h"
#include "shadermanager.h"

#include <../Smaug/transform.h>
#include <shared.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/geometric.hpp>
#include <bgfx/bgfx.h>

struct TexturedPlanePosColorVertex
{
	float x;
	float y;
	float z;
	float u;
	float v;

	static void init()
	{
		ms_layout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.end();
	}
	static bgfx::VertexLayout ms_layout;
};
bgfx::VertexLayout TexturedPlanePosColorVertex::ms_layout;

static TexturedPlanePosColorVertex s_texturedPlaneVertices[] =
{
	{-1.0f,  0.0f, -1.0f, 0.0f, 0.0f },
	{ 1.0f,  0.0f, -1.0f, 0.0f, 1.0f },
	{ 1.0f,  0.0f,  1.0f, 1.0f, 1.0f },
	{-1.0f,  0.0f,  1.0f, 1.0f, 0.0f },

};
static const uint16_t s_texturedPlaneTriList[] = { 2, 1, 0, 0, 3, 2 };


struct LineFormat
{
	static void init()
	{
		ms_layout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.end();
	}
	static bgfx::VertexLayout ms_layout;
};
bgfx::VertexLayout LineFormat::ms_layout;

static glm::vec3 s_lineVertices[] =
{
	{ -0.5, 0, 0},
	{  0.5, 0, 0},
	{  0.5, 0, 1},
	{ -0.5, 0, 1},


	{ 0, -0.5, 0},
	{ 0,  0.5, 0},
	{ 0,  0.5, 1},
	{ 0, -0.5, 1},
};
static const uint16_t s_lineTriList[] = { 0, 3, 2, 2, 1, 0, 4, 7, 6, 6, 5, 4 };


CBasicDraw::CBasicDraw()
{
	TexturedPlanePosColorVertex::init();
	LineFormat::init();


	m_planeVertexBuf = bgfx::createVertexBuffer(bgfx::makeRef(s_texturedPlaneVertices, sizeof(s_texturedPlaneVertices)), TexturedPlanePosColorVertex::ms_layout);
	m_planeIndexBuf = bgfx::createIndexBuffer(bgfx::makeRef(s_texturedPlaneTriList, sizeof(s_texturedPlaneTriList)));

	m_lineVertexBuf = bgfx::createVertexBuffer(bgfx::makeRef(s_lineVertices, sizeof(s_lineVertices)), LineFormat::ms_layout);
	m_lineIndexBuf = bgfx::createIndexBuffer(bgfx::makeRef(s_lineTriList, sizeof(s_lineTriList)));
	
	m_gridScale = bgfx::createUniform("gridScale", bgfx::UniformType::Vec4);
	m_gridDirMask = bgfx::createUniform("gridDirMask", bgfx::UniformType::Vec4);
}

CBasicDraw::~CBasicDraw()
{
	bgfx::destroy(m_planeVertexBuf);
	bgfx::destroy(m_planeIndexBuf);

	bgfx::destroy(m_lineVertexBuf);
	bgfx::destroy(m_lineIndexBuf);

	bgfx::destroy(m_gridScale);
	bgfx::destroy(m_gridDirMask);
}


void CBasicDraw::Cube(glm::mat4 mtx)
{
	bgfx::setTransform(&mtx[0][0]);
	bgfx::setVertexBuffer(0, m_lineVertexBuf);
	bgfx::setIndexBuffer(m_lineIndexBuf);

	ShaderManager().SetColor({1,0,0,1});
	bgfx::setState(0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_MSAA);

	bgfx::submit(ModelManager().CurrentView(), ShaderManager().GetShaderProgram(Shader::LINE));
}

void CBasicDraw::Plane(glm::mat4 mtx)
{
	bgfx::setTransform(&mtx[0][0]);
	bgfx::setVertexBuffer(0, m_planeVertexBuf);
	bgfx::setIndexBuffer(m_planeIndexBuf);
}

void CBasicDraw::Plane(glm::vec3 origin, glm::vec3 scale, glm::vec3 angle)
{
	glm::mat4 mtx = glm::identity<glm::mat4>();
	mtx = glm::translate(mtx, origin);
	mtx = glm::scale(mtx, scale);
	mtx *= glm::yawPitchRoll(angle.x, angle.y, angle.z);

	bgfx::setTransform(&mtx[0][0]);
	bgfx::setVertexBuffer(0, m_planeVertexBuf);
	bgfx::setIndexBuffer(m_planeIndexBuf);
}

void CBasicDraw::Line(glm::vec3 start, glm::vec3 end, glm::vec3 color, float width)
{
	CTransform mt;
	mt.SetLocalOrigin(start);
	mt.SetLocalScale(width, width, glm::distance(start, end));
	glm::vec3 ang = Angles(end - start);
	//ang.z = PI / 4.0f;
	mt.SetLocalAngles(ang);

	ShaderManager().SetColor(glm::vec4(color, 1.0f));
	bgfx::setTransform(&mt.Matrix()[0][0]);
	bgfx::setVertexBuffer(0, m_lineVertexBuf);
	bgfx::setIndexBuffer(m_lineIndexBuf);

	bgfx::setState(0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z 
		| BGFX_STATE_DEPTH_TEST_LESS 
		| BGFX_STATE_MSAA);

	bgfx::submit(ModelManager().CurrentView(), ShaderManager().GetShaderProgram(Shader::LINE));
}

void CBasicDraw::Grid(CTransform& transform, int scale)
{

	glm::vec4 vecScale = glm::vec4(1 / (float)(pow(2, scale)));

	glm::vec3 vecDir;
	Directions(transform.GetLocalAngles(), 0, 0, &vecDir);
	glm::vec4 dirMask = { 1.0f - fabs(vecDir.x), 1.0f - fabs(vecDir.y), 1.0f - fabs(vecDir.z), 1.0 };


	bgfx::setUniform(m_gridScale, &vecScale);
	bgfx::setUniform(m_gridDirMask, &dirMask);
	bgfx::setTransform(&transform.Matrix()[0][0]);
	bgfx::setVertexBuffer(0, m_planeVertexBuf);
	bgfx::setIndexBuffer(m_planeIndexBuf);
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

CBasicDraw& BasicDraw()
{
	static CBasicDraw s_basicDraw;
	return s_basicDraw;
}

