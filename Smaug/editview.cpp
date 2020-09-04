#include "editview.h"

#include "smaugapp.h"

#include <bigg.hpp>
#include <bx/string.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp> 
#include "worldeditor.h"
#include "worldrenderer.h"
#include "utils.h"
#include "shadermanager.h"

/*
 * Snagged from Bigg's examples
 *
 * Simple rotating cubes example, based off bgfx's example-01-cubes.
 * Does not mimic the original example completely because we're in a
 * different coordinate system.
 *
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

static struct PosColorVertex
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
bgfx::VertexLayout PosColorVertex::ms_layout;

static PosColorVertex s_cubeVertices[] =
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

#define SCROLL_SPEED 5

void CEditView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);

	PosColorVertex::init();

	m_hShaderProgram = ShaderManager::GetShaderProgram(Shader::EDIT_VIEW_SHADER);
	m_hVertexBuf = bgfx::createVertexBuffer(bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)), PosColorVertex::ms_layout);
	m_hIndexBuf = bgfx::createIndexBuffer(bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList)));
	

	CNode* node = GetWorldEditor().CreateQuad();

	node->m_origin = glm::vec3(0, 0, -20);

	for (int i = 0; i < node->m_sideCount; i++)
		node->m_sides[i].vertex1->origin *= 10;
	node->Update();

	// Zoom and pan
	m_viewportHeight = m_viewportWidth = 80;
	m_cameraPos = glm::vec3(0, 10, 0);

	m_cursorSpin = 1;
	GetActionManager().actionMode = ActionMode::NONE;
}

float t = 0;

void CEditView::Update(float dt, float mx, float my)
{
	float scrollDelta = ImGui::GetIO().MouseWheel * SCROLL_SPEED;
	m_viewportWidth  -= scrollDelta;
	m_viewportHeight -= scrollDelta;
	// Offset the cam with the scrollDelta!

	// Put the mouse pos into the world
	mx = (mx * 2 - 1) * m_viewportWidth - m_cameraPos.x;
	my = (my * 2 - 1) * m_viewportHeight - m_cameraPos.z;

	glm::mat4 mtx;
 
	m_mousePos = glm::vec3(mx, 5, my);

	m_cursorSpin = 1;

	GetActionManager().Act(m_mousePos);


}

void CEditView::Draw(float dt)
{
	t += dt * m_cursorSpin;
	CBaseView::Draw(dt);


	// Camera
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::rotate(view, glm::radians(90.0f), glm::vec3(1, 0, 0)); // Look down
	view = glm::translate(view, m_cameraPos); // Set our pos to be 10 units up
	glm::mat4 proj = glm::ortho(-m_viewportWidth, m_viewportWidth, -m_viewportHeight, m_viewportHeight, -900.0f, 800.0f);
	bgfx::setViewTransform(m_viewId, &view[0][0], &proj[0][0]);


	GetWorldRenderer().Draw2D(m_viewId, Shader::EDIT_VIEW_SHADER);


	for (int i = 0; i < GetWorldEditor().m_nodes.size(); i++)
	{
		glm::mat4 mtx = glm::identity<glm::mat4>();
		mtx = glm::translate(mtx, GetWorldEditor().m_nodes[i]->m_origin);
		mtx = glm::scale(mtx, glm::vec3(2.5f, 2.5f, 2.5f));
		mtx *= glm::yawPitchRoll(1.37f * t, t, 0.0f);
		bgfx::setTransform(&mtx[0][0]);
		bgfx::setVertexBuffer(0, m_hVertexBuf);
		bgfx::setIndexBuffer(m_hIndexBuf);
		bgfx::setState(BGFX_STATE_DEFAULT);
		bgfx::submit(m_viewId, m_hShaderProgram);
	}


	// Cursor
	glm::mat4 mtx = glm::identity<glm::mat4>();
	mtx = glm::translate(mtx, GetActionManager().cursorPos);
	mtx = glm::scale(mtx, glm::vec3(2.5f, 2.5f, 2.5f));
	mtx *= glm::yawPitchRoll(1.37f * t, t, 0.0f);
	bgfx::setTransform(&mtx[0][0]);
	bgfx::setVertexBuffer(0, m_hVertexBuf);
	bgfx::setIndexBuffer(m_hIndexBuf);
	bgfx::setState(BGFX_STATE_DEFAULT);
	bgfx::submit(m_viewId, m_hShaderProgram);

}