#include "basicdraw.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/geometric.hpp>

/*
 * Cube rendering snagged from Bigg's examples
 *
 * Simple rotating cubes example, based off bgfx's example-01-cubes.
 * Does not mimic the original example completely because we're in a
 * different coordinate system.
 *
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

struct CubePosColorVertex
{
	float x;
	float y;
	float z;
	uint32_t abgr;
	static void init()
	{
		ms_layout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
	}
	static bgfx::VertexLayout ms_layout;
};
bgfx::VertexLayout CubePosColorVertex::ms_layout;

static CubePosColorVertex s_cubeVertices[] =
{
	{-1.0f,  1.0f,  1.0f, 0xff000000 },
	{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
	{-1.0f, -1.0f,  1.0f, 0xff00ff00 },
	{ 1.0f, -1.0f,  1.0f, 0xff00ffff },
	{-1.0f,  1.0f, -1.0f, 0xffff0000 },
	{ 1.0f,  1.0f, -1.0f, 0xffff00ff },
	{-1.0f, -1.0f, -1.0f, 0xffffff00 },
	{ 1.0f, -1.0f, -1.0f, 0xffffffff },
};
static const uint16_t s_cubeTriList[] = { 2, 1, 0, 2, 3, 1, 5, 6, 4, 7, 6, 5, 4, 2, 0, 6, 2, 4, 3, 5, 1, 3, 7, 5, 1, 4, 0, 1, 5, 4, 6, 3, 2, 7, 3, 6 };


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


CBasicDraw::CBasicDraw()
{
	CubePosColorVertex::init();
	TexturedPlanePosColorVertex::init();

	m_cubeVertexBuf = bgfx::createVertexBuffer(bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)), CubePosColorVertex::ms_layout);
	m_cubeIndexBuf = bgfx::createIndexBuffer(bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList)));

	m_planeVertexBuf = bgfx::createVertexBuffer(bgfx::makeRef(s_texturedPlaneVertices, sizeof(s_texturedPlaneVertices)), TexturedPlanePosColorVertex::ms_layout);
	m_planeIndexBuf = bgfx::createIndexBuffer(bgfx::makeRef(s_texturedPlaneTriList, sizeof(s_texturedPlaneTriList)));
	
}


void CBasicDraw::Cube(glm::mat4 mtx)
{
	bgfx::setTransform(&mtx[0][0]);
	bgfx::setVertexBuffer(0, m_cubeVertexBuf);
	bgfx::setIndexBuffer(m_cubeIndexBuf);
	//bgfx::setState(BGFX_STATE_DEFAULT);
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

CBasicDraw& BasicDraw()
{
	static CBasicDraw s_basicDraw;
	return s_basicDraw;
}

